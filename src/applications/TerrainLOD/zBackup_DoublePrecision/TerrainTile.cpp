
#include <applications/Camera/Camera.h>
#include <applications/TerrainLOD/zBackup_DoublePrecision/LODSelection.h>
#include <applications/TerrainLOD/zBackup_DoublePrecision/QuadTree.h>
#include <applications/TerrainLOD/zBackup_DoublePrecision/TerrainTile.h>
#include <base/Globals.h>
#include <base/Logbook.h>
#include <omath/mat4.h>
#include <renderer/Color.h>
#include <renderer/Module.h>
#include <renderer/Uniform.h>
#include <renderer/VertexArray3D.h>
#include <scene/Scene.h>
#include <iostream>
#include <string>
#include "Terrain/GridMesh.h"
#include <sstream>

namespace terrain {

TerrainTile::TerrainTile( const std::string &filename ) : m_filename{ filename } {
	// Load the heightmap and tile relative and world min/max coords for the bounding boxes
	// @todo: check if size == terrain::TILE_SIZE !
	m_heightMap = new HeightMap{ filename + ".png", terrain::HeightMap::B16, HEIGHTMAP_UNIT };
	std::ifstream bbf{ filename + ".bb", std::ios::in };
	if( !bbf.is_open() ) {
		std::ostringstream s;
		s << "Error opening bounding box file '" << filename << ".bb'. Tile will not be rendered correctly.";
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s.str() );
	}
	omath::dvec3 min, max;
	bbf >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
	omath::dvec3 cartesianMin, cartesianMax;
	bbf >> m_lon >> m_lat >> m_cellSize;
	bbf.close();
	// bb relative to heightmap
	m_AABB = new orf_n::dAABB{
		omath::dvec3{ min.x, min.y * HEIGHT_FACTOR, min.z },
		omath::dvec3{ max.x, max.y * HEIGHT_FACTOR, max.z }
	};
	if( 0.0 >= m_cellSize || omath::compareFloat( omath::magnitudeSq( m_AABB->getSize() ), 0.0 ) ||
		omath::compareFloat( m_lon, 0.0 ) || omath::compareFloat( m_lat, 0.0 ) ) {
		std::string s{ "Error reading bounding box data from '" + filename + ".bb'. Tile will not be rendered correctly." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
	}

	// Build quadtree with nodes and their bounding boxes.
	m_quadTree = new QuadTree{ m_heightMap, min, max, m_AABB };

	std::ostringstream s;
	s << "Terrain tile '" << filename << "'loaded.";
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
	s.str( std::string() );
	s << "\tBounding box of whole tile: " << *m_AABB;
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
	// Clear the stream
	s.str( std::string() );
	s << std::fixed << "\tGeodetic coordinates of tile: " << m_lon << "°lon/" << m_lat << "°lat; cellsize " << m_cellSize;
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
	s.str( std::string() );
}

TerrainTile::~TerrainTile() {
	delete m_quadTree;
	delete m_heightMap;
	delete m_AABB;
	std::string s{ "Terrain tile '" + m_filename + "'unloaded." };
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s );
}

const terrain::HeightMap *TerrainTile::getHeightMap() const {
	return m_heightMap;
}

const terrain::QuadTree *TerrainTile::getQuadTree() const {
	return m_quadTree;
}

const orf_n::dAABB *TerrainTile::getAABB() const {
	return m_AABB;
}

const omath::dvec2 TerrainTile::getLonLat() const {
	return omath::dvec2{ m_lon, m_lat };
}

const double &TerrainTile::getCellSize() const {
	return m_cellSize;
}

omath::uvec2 TerrainTile::render( const orf_n::Program *const p,
						  const terrain::GridMesh *const gridMesh,
						  const terrain::LODSelection *const selection,
						  const int tileIndex,
						  const GLint drawMode ) {
	m_heightMap->bind();
	// Submeshes are evenly spaced in index buffer. Else calc offsets individually.
	const int halfD{ gridMesh->getEndIndexTL() };

	// Iterate through the lod selection's lod levels
	omath::uvec2 renderStats{0};
	for( int i{ selection->m_minSelectedLODLevel }; i <= selection->m_maxSelectedLODLevel; ++i ) {
		const int filterLODLevel{ i };
		// @todo: gridmesh is originally picked here. Why ?
		int prevMorphConstLevelSet{ -1 };
		for( int i{ 0 }; i < selection->m_selectionCount; ++i ) {
			const LODSelection::selectedNode_t &n{ selection->m_selectedNodes[i] };
			// Only draw tiles of the currently bound heightmap
			if( n.tileIndex != tileIndex )
				continue;
			// Filter out the node if not of the current level
			if( filterLODLevel != -1 && filterLODLevel != n.lodLevel )
				continue;
			// Set LOD level specific consts if they have changed from last lod level
			if( prevMorphConstLevelSet != n.lodLevel ) {
				prevMorphConstLevelSet = n.lodLevel;
				orf_n::setUniform( p->getProgram(), "g_morphConsts",
						selection->getMorphConsts( prevMorphConstLevelSet-1 ) );
			}
			bool drawFull{ n.hasTL && n.hasTR && n.hasBL && n.hasBR };
			const orf_n::dAABB *bb{ n.node->getBoundingBox() };
			omath::vec4 nodeScale{ bb->getSize().x, 0.0f, bb->getSize().z, static_cast<float>(n.lodLevel) };
			omath::vec3 nodeOffset{ bb->m_min.x, bb->m_min.y + bb->m_max.y * 0.5f, bb->m_min.z };
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
