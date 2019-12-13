
#include <applications/Camera/Camera.h>
#include <applications/WorldTerrainLOD/GridMesh.h>
#include <applications/WorldTerrainLOD/LODSelection.h>
#include <applications/WorldTerrainLOD/QuadTree.h>
#include <applications/WorldTerrainLOD/TerrainTile.h>
#include <base/Logbook.h>
#include <geometry/Ellipsoid.h>
#include <omath/mat4.h>
#include <renderer/Module.h>
#include <renderer/Sampler.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>
#include <iostream>
#include <string>
#include <sstream>

namespace terrain {

TerrainTile::TerrainTile( const std::string &filename, const orf_n::Ellipsoid *const ellipsoid ) :
		m_filename{filename}, m_ellipsoid{ellipsoid} {
	// Load the heightmap and tile relative and world min/max coords for the bounding boxes
	// @todo: check if size == terrain::TILE_SIZE !
	m_heightMap = std::make_unique<HeightMap>( filename + ".png", terrain::HeightMap::B16 );
	std::ifstream bbf{ filename + ".bb", std::ios::in };
	if( !bbf.is_open() ) {
		std::ostringstream s;
		s << "Error opening bounding box file '" << filename << ".bb'. Tile will not be rendered correctly.";
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s.str() );
	}
	omath::dvec3 min, max;
	bbf >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
	bbf >> m_lon >> m_lat >> m_cellsize;
	bbf.close();
	const omath::dvec3 cartesianMin{ m_ellipsoid->toCartesian( { omath::radians(m_lat), omath::radians(m_lon), 0.0 } ) };
	const omath::dvec3 cartesianMax{ m_ellipsoid->toCartesian( {
		omath::radians( m_lat + m_heightMap->getExtent().y * m_cellsize ),
		omath::radians( m_lon + m_heightMap->getExtent().x * m_cellsize ), 0.0
	} ) };
	// bb relative to heightmap @todo ditch the factor !
	m_AABB = std::make_unique<orf_n::AABB>( cartesianMin, cartesianMax );
	if( 0.0 >= m_cellsize || omath::compareFloat( omath::magnitudeSq( m_AABB->getSize() ), 0.0 ) ||
		omath::compareFloat( m_lon, 0.0 ) || omath::compareFloat( m_lat, 0.0 ) ) {
		std::string s{ "Error reading bounding box data from '" + filename + ".bb'. Tile will not be rendered correctly." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
	}
	// Build quadtree with nodes and their bounding boxes.
	m_quadTree = std::make_unique<QuadTree>( this );
	std::ostringstream s;
	s << "Terrain tile '" << filename << "'loaded.";
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
	s.str( std::string() );
	s << "\tBounding box of whole tile: " << *m_AABB << "; Size of coordinate textures: " <<
			static_cast<float>( m_heightMap->getExtent().x * m_heightMap->getExtent().y * 6 * sizeof(GLfloat) ) / 1024.0f
			<< "kb" ;
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
	// Clear the stream
	s.str( std::string() );
	s << std::fixed << "\tGeodetic coordinates of tile: " << m_lat << "°lat/" << m_lon << "°lon; cellsize " << m_cellsize;
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
}

TerrainTile::~TerrainTile() {
	std::string s{ "Terrain tile '" + m_filename + "'unloaded." };
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s );
}

const QuadTree *TerrainTile::getQuadTree() const {
	return m_quadTree.get();
}

const HeightMap *TerrainTile::getHeightMap() const {
	return m_heightMap.get();
}

const orf_n::AABB *TerrainTile::getAABB() const {
	return m_AABB.get();
}

const omath::dvec2 TerrainTile::getLatLon() const {
	return omath::dvec2{ m_lat, m_lon };
}

const orf_n::Ellipsoid *TerrainTile::getEllipsoid() const {
	return m_ellipsoid;
}

const double &TerrainTile::getCellSize() const {
	return m_cellsize;
}

omath::uvec2 TerrainTile::render( const orf_n::Program *const p,
						  const terrain::GridMesh *const gridMesh,
						  const terrain::LODSelection *const selection,
						  const int tileIndex,
						  const GLint drawMode ) {
	omath::uvec2 renderStats{0};
	m_heightMap->bind();
	// Set cellsize and lower left lat/lon to calculate world coords from tile coords in shader
	orf_n::setUniform( p->getProgram(), "cellsize", static_cast<float>( m_cellsize ) );
	orf_n::setUniform( p->getProgram(), "worldLowerLeft", omath::vec2( static_cast<float>(m_lat), static_cast<float>(m_lon) ) );
	// Submeshes are evenly spaced in index buffer. Else calc offsets individually.
	const int halfD{ gridMesh->getEndIndexTL() };
	// Iterate through the lod selection's lod levels
	for( int i{ selection->m_minSelectedLODLevel }; i <= selection->m_maxSelectedLODLevel; ++i ) {
		const int filterLODLevel{ i };
		// @todo: gridmesh is originally picked here. Why ?
		int prevMorphConstLevelSet{ -1 };
		for( int i{ 0 }; i < selection->m_selectionCount; ++i ) {
			const LODSelection::selectedNode_t &n{ selection->m_selectedNodes[i] };
			// Only draw tiles of the currently bound heightmap; filter out nodes if not of the current level
			if( n.tileIndex != tileIndex || ( filterLODLevel != -1 && filterLODLevel != n.lodLevel ) )
				continue;
			// Set LOD level specific consts if they have changed from last lod level
			if( prevMorphConstLevelSet != n.lodLevel ) {
				prevMorphConstLevelSet = n.lodLevel;
				orf_n::setUniform( p->getProgram(), "u_morphConsts",
						selection->getMorphConsts( prevMorphConstLevelSet-1 ) );
			}
			bool drawFull{ n.hasTL && n.hasTR && n.hasBL && n.hasBR };
			// For drawing, we need the bounding boxes relative to the tile being drawn
			const orf_n::AABB *const tileBB{ n.node->getTileBoundingBox() };
			const orf_n::AABB *const worldBB{ n.node->getWorldBoundingBox() };
			// .w holds the current lod level
			omath::vec4 nodeScale{
				static_cast<float>(tileBB->getSize().x), 0.0f, static_cast<float>(tileBB->getSize().z), static_cast<float>(n.lodLevel)
			};
			omath::vec3 nodeOffset{ static_cast<float>( tileBB->m_min.x ),
									static_cast<float>( tileBB->m_min.y ) + static_cast<float>( tileBB->m_max.y ) * 0.5f,
									static_cast<float>( tileBB->m_min.z ) };
			// ******************* @todo
			const omath::ivec2 nodeOrg{ n.node->getXZ() };
			orf_n::Geodetic geodeticMin{
				omath::radians( m_lat + nodeOrg.y * m_cellsize ),
				omath::radians( m_lon + nodeOrg.x * m_cellsize ),
				m_heightMap->getHeightAt( nodeOrg.x, nodeOrg.y ) * 100
			};
			omath::dvec3 posMin{ m_ellipsoid->toCartesian( geodeticMin ) };

			// *******************
			orf_n::setUniform( p->getProgram(), "u_nodeScale", nodeScale );
			orf_n::setUniform( p->getProgram(), "u_nodeOffset", nodeOffset );
			orf_n::setUniform( p->getProgram(), "u_nodeNormal", nodeOffset );
			const int numIndices{ gridMesh->getNumberOfIndices() };
			if( drawFull ) {
				glDrawElements( drawMode, numIndices, GL_UNSIGNED_INT, (const void *)0 );
				++renderStats.x;
				renderStats.y += numIndices / 3;
			} else {
				// can be optimized by combining calls
				if( n.hasTL ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT, (const void *)0 );
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
				if( n.hasTR ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT,
							(const void *)( gridMesh->getEndIndexTL() * sizeof( GL_UNSIGNED_INT ) ) );
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
				if( n.hasBL ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT,
							(const void *)( gridMesh->getEndIndexTR() * sizeof( GL_UNSIGNED_INT ) ));
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
				if( n.hasBR ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT,
							(const void *)( gridMesh->getEndIndexBL() * sizeof( GL_UNSIGNED_INT ) ) );
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
			}
		}
	}
	return renderStats;
}

}
