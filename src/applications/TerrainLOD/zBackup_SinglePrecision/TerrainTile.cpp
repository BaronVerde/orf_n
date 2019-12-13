
#include <applications/Camera/Camera.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/LODSelection.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/QuadTree.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/TerrainTile.h>
#include <base/Globals.h>
#include <base/Logbook.h>
#include <omath/mat4.h>
#include <renderer/Color.h>
#include <renderer/Module.h>
#include <renderer/Uniform.h>
#include <renderer/VertexArray3D.h>
#include <scene/Scene.h>
#include <iostream>
#include "Terrain/GridMesh.h"
#include <sstream>

namespace terrain {

TerrainTile::TerrainTile( const std::string &filename ) : m_filename{ filename } {
	// Load the heightmap and world min/max coords for the bounding box
	m_heightMap = new HeightMap{ filename + ".png", terrain::HeightMap::B16, HEIGHTMAP_UNIT };
	std::ifstream bbf{ filename + ".bb", std::ios::in };
	if( !bbf.is_open() ) {
		std::ostringstream s;
		s << "Error opening bounding box file '" << filename << ".bb'. Tile will not be rendered correctly.";
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s.str() );
	}
	omath::dvec3 min, max;
	bbf >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
	bbf.close();

	// Build quadtree with nodes and their bounding boxes.
	m_quadTree = new QuadTree{ m_heightMap, min, max };

	const omath::vec3 tileMax{ m_quadTree->getTileBoundingBox()->m_max };
	const omath::vec3 tileMin{ m_quadTree->getTileBoundingBox()->m_min };
	m_quadWorldMax = omath::vec2{ tileMax.x, tileMax.z };
	//m_terrainScale = omath::vec3{ worldMax.x - worldMin.x, worldMax.y - worldMin.y, worldMax.z - worldMin.z };
	m_terrainScale = omath::vec3{ tileMax - tileMin };
	m_terrainOffset = tileMin;
	std::string s{ "Terrain tile '" + filename + "'loaded." };
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s );
}

TerrainTile::~TerrainTile() {
	delete m_quadTree;
	delete m_heightMap;
	std::string s{ "Terrain tile '" + m_filename + "'unloaded." };
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s );
}

const terrain::HeightMap *TerrainTile::getHeightMap() const {
	return m_heightMap;
}

const terrain::QuadTree *TerrainTile::getQuadTree() const {
	return m_quadTree;
}

const orf_n::AABB *TerrainTile::getTileBoundingBox() const {
	return m_quadTree->getTileBoundingBox();
}

void TerrainTile::render( const orf_n::Program *const p,
						  const terrain::GridMesh *const gridMesh,
						  const terrain::LODSelection *const selection,
						  const int tileIndex,
						  const GLint drawMode ) {
	orf_n::setUniform( p->getProgram(), "g_quadWorldMax", m_quadWorldMax );
	orf_n::setUniform( p->getProgram(), "g_terrainScale", m_terrainScale );
	orf_n::setUniform( p->getProgram(), "g_terrainOffset", m_terrainOffset );

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
			// Only draw tiles of the currently bound heightmap
			if( n.tileIndex != tileIndex )
				continue;
			// Filter out the node if required
			if( filterLODLevel != -1 && filterLODLevel != n.lodLevel )
				continue;
			// Set LOD level specific consts if they have changed from last lod level
			if( prevMorphConstLevelSet != n.lodLevel ) {
				prevMorphConstLevelSet = n.lodLevel;
				orf_n::setUniform( p->getProgram(), "g_morphConsts",
						selection->getMorphConsts( prevMorphConstLevelSet-1 ) );
			}
			bool drawFull{ n.hasTL && n.hasTR && n.hasBL && n.hasBR };
			const orf_n::AABB *bb{ n.node->getBoundingBox() };
			omath::vec4 nodeScale{ bb->getSize().x, 0.0f, bb->getSize().z, (float)n.lodLevel };
			omath::vec3 nodeOffset{ bb->m_min.x, bb->m_min.y + bb->m_max.y * 0.5f, bb->m_min.z };
			//omath::vec3 nodeOffset{ bb->getMin().x, 0.0f, bb->getMin().z };
			orf_n::setUniform( p->getProgram(), "g_nodeScale", nodeScale );
			orf_n::setUniform( p->getProgram(), "g_nodeOffset", nodeOffset );
			int renderedTriangles{ 0 };

			const int numIndices{ gridMesh->getNumberOfIndices() };
			if( drawFull ) {
				glDrawElements( drawMode, numIndices, GL_UNSIGNED_INT, (const void *)0 );
				renderedTriangles += numIndices / 3;
			} else {
				// can be optimized by combining calls
				if( n.hasTL ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT, (const void *)0 );
					renderedTriangles += halfD / 3;
				}
				if( n.hasTR ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT,
							(const void *)( gridMesh->getEndIndexTL() * sizeof( GL_UNSIGNED_INT ) ) );
					renderedTriangles += halfD / 3;
				}
				if( n.hasBL ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT,
							(const void *)( gridMesh->getEndIndexTR() * sizeof( GL_UNSIGNED_INT ) ));
					renderedTriangles += halfD / 3;
				}
				if( n.hasBR ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT,
							(const void *)( gridMesh->getEndIndexBL() * sizeof( GL_UNSIGNED_INT ) ) );
					renderedTriangles += halfD / 3;
				}
			}
			/* @todo m_renderStats.totalRenderedQuads++;
			m_renderStats.totalRenderedTriangles += renderedTriangles;*/
		}
	}
}

}
