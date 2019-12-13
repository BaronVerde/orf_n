
#pragma once

#include <applications/Camera/Camera.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/HeightMap.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/LODSelection.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/Node.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/Settings.h>
#include <geometry/AABB.h>
#include <geometry/OBB.h>
#include <omath/vec3.h>
#include <cstring>

namespace terrain {

// @todo: needs overwork. Much data is meaningless
class QuadTree {
public:
	QuadTree( const terrain::HeightMap *const heightMap,
			  const omath::dvec3 &min,
			  const omath::dvec3 &max,
			  const orf_n::AABB *const tileBoundingBox );

	virtual ~QuadTree();

	const Node *getNodes() const;

	int getNodeCount() const;

	// tile index is saved in selection list for sorting by tile and distance
	void lodSelect( LODSelection *lodSelectlion ) const;

private:
	int m_rasterSizeX{ 0 };

	int m_rasterSizeZ{ 0 };

	int m_topNodeSize{ 0 };

	int m_topNodeCountX{ 0 };

	int m_topNodeCountZ{ 0 };

	int m_nodeCount{ 0 };

	Node *m_allNodes{ nullptr };

	Node ***m_topLevelNodes{ nullptr };

};

}
