
#include <applications/Camera/Camera.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/GridMesh.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/LODSelection.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/QuadTree.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/TerrainTile.h>
#include <base/Logbook.h>
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
		m_filename{ filename } {
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
	bbf >> m_lon >> m_lat >> m_cellSize;
	bbf.close();
	// bb relative to heightmap @todo ditch the factor !
	m_AABB = std::make_unique<orf_n::AABB>(
			omath::dvec3{ min.x, min.y, min.z },
			omath::dvec3{ max.x, max.y, max.z }
	);
	if( 0.0 >= m_cellSize || omath::compareFloat( omath::magnitudeSq( m_AABB->getSize() ), 0.0 ) ||
		omath::compareFloat( m_lon, 0.0 ) || omath::compareFloat( m_lat, 0.0 ) ) {
		std::string s{ "Error reading bounding box data from '" + filename + ".bb'. Tile will not be rendered correctly." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
	}

	// Build quadtree with nodes and their bounding boxes.
	m_quadTree = std::make_unique<QuadTree>( m_heightMap.get(), min, max, m_AABB.get() );

	buildPositionTextures( ellipsoid );

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
	s << std::fixed << "\tGeodetic coordinates of tile: " << m_lat << "°lat/" << m_lon << "°lon; cellsize " << m_cellSize;
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
}

void TerrainTile::buildPositionTextures( const orf_n::Ellipsoid *ellipsoid ) {
	// Build the textures to hold high and low cartesian positions
	// @todo: this can be simplified later
	int numPositions{ m_heightMap->getExtent().y * m_heightMap->getExtent().x };
	std::vector<omath::dvec3> cartesianPositions( numPositions );
	int count{0};
	for( int i{0}; i < m_heightMap->getExtent().y; ++i ) {
		const double currentLat{ m_lat + static_cast<double>(i) * m_cellSize };
		for( int j{0}; j < m_heightMap->getExtent().x; ++j ) {
			const double currentLon{ m_lon + static_cast<double>(j) * m_cellSize };
			// Height set to 0.0 at this point
			const orf_n::Geodetic geodeticPos{ omath::radians( currentLat ), omath::radians( currentLon ), 0.0 };
			cartesianPositions[count++] = ellipsoid->toCartesian( geodeticPos );
		}
	}
	count = 0;
	// Using vec3 instead of float for abbreviated writing. Data is reinterpreted to float below.
	std::vector<omath::vec3> cartesianPositionsHigh( numPositions );
	std::vector<omath::vec3> cartesianPositionsLow( numPositions );
	for( const omath::dvec3 &v : cartesianPositions ) {
		omath::doubleToTwoFloats( v, cartesianPositionsHigh[count], cartesianPositionsLow[count] );
		++count;
	}
	m_positionsHigh = std::make_unique<orf_n::Texture2D>(
		reinterpret_cast<GLfloat*>( cartesianPositionsHigh.data() ), m_heightMap->getExtent(), 1, false
	);
	orf_n::setDefaultSampler( m_positionsHigh->getName(), orf_n::NEAREST_CLAMP );
	m_positionsLow = std::make_unique<orf_n::Texture2D>(
		reinterpret_cast<GLfloat*>( cartesianPositionsLow.data() ), m_heightMap->getExtent(), 2, false
	);
	orf_n::setDefaultSampler( m_positionsLow->getName(), orf_n::NEAREST_CLAMP );

	m_worldCenter = ellipsoid->toCartesian(
			{ omath::radians( m_lat + m_heightMap->getExtent().y * 0.5 * m_cellSize ),
			  omath::radians( m_lon + m_heightMap->getExtent().x * 0.5 * m_cellSize ), 0.0 } );
}

TerrainTile::~TerrainTile() {
	std::string s{ "Terrain tile '" + m_filename + "'unloaded." };
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s );
}

const terrain::HeightMap *TerrainTile::getHeightMap() const {
	return m_heightMap.get();
}

const orf_n::Texture2D *TerrainTile::getPositionsHigh() const {
	return m_positionsHigh.get();
}

const orf_n::Texture2D *TerrainTile::getPositionsLow() const {
	return m_positionsLow.get();
}

const terrain::QuadTree *TerrainTile::getQuadTree() const {
	return m_quadTree.get();
}

const orf_n::AABB *TerrainTile::getAABB() const {
	return m_AABB.get();
}

const omath::dvec2 TerrainTile::getLatLon() const {
	return omath::dvec2{ m_lat, m_lon };
}

const double &TerrainTile::getCellSize() const {
	return m_cellSize;
}

const omath::dvec3 &TerrainTile::getWorldCenter() const {
	return m_worldCenter;
}

omath::uvec2 TerrainTile::render( const orf_n::Program *const p,
						  const terrain::GridMesh *const gridMesh,
						  const terrain::LODSelection *const selection,
						  const int tileIndex,
						  const GLint drawMode ) {
	omath::uvec2 renderStats{0};
	m_heightMap->bind();
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
				orf_n::setUniform( p->getProgram(), "g_morphConsts",
						selection->getMorphConsts( prevMorphConstLevelSet-1 ) );
			}
			bool drawFull{ n.hasTL && n.hasTR && n.hasBL && n.hasBR };
			const orf_n::AABB *const bb{ n.node->getBoundingBox() };
			// .w holds the current lod level
			omath::vec4 nodeScale{
				static_cast<float>( bb->getSize().x ), 0.0f, static_cast<float>( bb->getSize().z ),
						static_cast<float>( n.lodLevel )
			};
			omath::vec3 nodeOffset{ static_cast<float>( bb->m_min.x ),
									static_cast<float>( bb->m_min.y ) + static_cast<float>( bb->m_max.y ) * 0.5f,
									static_cast<float>( bb->m_min.z ) };
			orf_n::setUniform( p->getProgram(), "g_nodeScale", nodeScale );
			orf_n::setUniform( p->getProgram(), "g_nodeOffset", nodeOffset );
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
