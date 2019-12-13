
#pragma once

#include <applications/Camera/Camera.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/LODSelection.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/Node.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/Settings.h>
#include <geometry/AABB.h>
#include <geometry/OBB.h>
#include <omath/vec3.h>
#include "Terrain/HeightMap.h"
#include <cstring>

namespace terrain {

class QuadTree {
public:
	QuadTree( const terrain::HeightMap *heightMap, const omath::dvec3 &min, const omath::dvec3 &max );

	virtual ~QuadTree();

	const Node *getNodes() const;

	int getNodeCount() const;

	// tile index is saved in selection list for sorting by tile and distance
	void lodSelect( LODSelection *lodSelectlion ) const;

	const orf_n::AABB *getTileBoundingBox() const;

private:
	const terrain::HeightMap *m_heightMap{ nullptr };

	float m_leafNodeWorldSizeX{ 0.0f };

	float m_leafNodeWorldSizeZ{ 0.0f };

	int m_rasterSizeX{ 0 };

	int m_rasterSizeZ{ 0 };

	int m_topNodeSize{ 0 };

	int m_topNodeCountX{ 0 };

	int m_topNodeCountZ{ 0 };

	int m_nodeCount{ 0 };

	float m_nodeDiagonalSizes[NUMBER_OF_LOD_LEVELS];

	Node *m_allNodes{ nullptr };

	Node ***m_topLevelNodes{ nullptr };

	/**
	 * Bounding box relative to tile
	 */
	const orf_n::AABB *m_tileBoundingBox{ nullptr };

	/**
	 * Bounding box in cartesian world coordinates
	 */
	const orf_n::OBB *m_OBB{ nullptr };

};

}
