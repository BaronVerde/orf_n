
#include <applications/camera/camera.h>
#include <applications/terrain_lod/gridmesh.h>
#include <applications/terrain_lod/LODSelection.h>
#include <applications/terrain_lod/quadtree.h>
#include <applications/terrain_lod/TerrainTile.h>
#include <base/logbook.h>
#include <renderer/sampler.h>
#include <scene/scene.h>
#include "omath/mat4.h"
#include "renderer/Module.h"
#include "renderer/Uniform.h"
#include <iostream>
#include <string>
#include <sstream>

namespace terrain {

TerrainTile::TerrainTile( const std::string &filename ) :
		m_filename{filename} {
	// Load the heightmap and tile relative and world min/max coords for the bounding boxes
	// @todo: check if size == terrain::TILE_SIZE !
	m_heightMap = std::make_unique<heightmap>( filename + ".png", terrain::heightmap::B16 );
	std::ifstream bbf{ filename + ".bb", std::ios::in };
	if( !bbf.is_open() ) {
		std::ostringstream s;
		s << "Error opening bounding box file '" << filename << ".bb'. Tile will not be rendered correctly.";
		orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::WARNING, s.str() );
	}
	omath::dvec3 min, max;
	bbf >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
	bbf.close();
	//min = orf_n::
	m_AABB = std::make_unique<orf_n::aabb>( omath::dvec3{ min.x, min.y, min.z }, omath::dvec3{ max.x, max.y, max.z } );

	// Build quadtree with nodes and their bounding boxes.
	m_quadTree = std::make_unique<quad_tree>( this );

	std::ostringstream s;
	s << "Terrain tile '" << filename << "'loaded.";
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO, s.str() );
	s.str( std::string() );
	s << "\tBounding box of whole tile: " << *m_AABB;
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO, s.str() );
}

TerrainTile::~TerrainTile() {
	std::string s{ "Terrain tile '" + m_filename + "'unloaded." };
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO, s );
}

const terrain::heightmap *TerrainTile::getHeightMap() const {
	return m_heightMap.get();
}

const terrain::quad_tree *TerrainTile::getQuadTree() const {
	return m_quadTree.get();
}

const orf_n::aabb *TerrainTile::getAABB() const {
	return m_AABB.get();
}

omath::uvec2 TerrainTile::render( const orf_n::Program *const p,
						  const terrain::gridmesh *const gridMesh,
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
			const orf_n::aabb *const bb{ n.p_node->getBoundingBox() };
			// .w holds the current lod level
			omath::vec4 nodeScale{
				static_cast<float>( bb->get_size().x ), 0.0f, static_cast<float>( bb->get_size().z ),
				static_cast<float>( n.lodLevel )
			};
			omath::vec3 nodeOffset{ static_cast<float>( bb->m_min.x ),
									static_cast<float>( bb->m_min.y ) + static_cast<float>( bb->m_max.y ) * 0.5f,
									static_cast<float>( bb->m_min.z ) };
			orf_n::setUniform( p->getProgram(), "g_nodeScale", nodeScale );
			orf_n::setUniform( p->getProgram(), "g_nodeOffset", nodeOffset );
			const int numIndices{ gridMesh->get_number_indices() };
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
