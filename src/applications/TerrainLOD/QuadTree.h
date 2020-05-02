
#pragma once

#include <applications/camera/camera.h>
#include <applications/TerrainLOD/heightmap.h>
#include "LODSelection.h"
#include "Node.h"
#include "omath/vec3.h"

namespace terrain {

class TerrainTile;

// @todo: needs overwork. Much data is meaningless
class QuadTree {
public:
	QuadTree( const TerrainTile *const terrainTile );

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

	const TerrainTile *const m_terrainTile{nullptr};

};

}
