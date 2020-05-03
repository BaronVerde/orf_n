
#pragma once

#include <applications/camera/camera.h>
#include <applications/terrain_lod/heightmap.h>
#include <applications/terrain_lod/LODSelection.h>
#include <applications/terrain_lod/node.h>
#include "omath/vec3.h"

namespace terrain {

class TerrainTile;

// @todo: needs overwork. Much data is meaningless
class quad_tree {
public:
	quad_tree( const TerrainTile *const terrainTile );

	virtual ~quad_tree();

	const node *getNodes() const;

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

	node *m_allNodes{ nullptr };

	node ***m_topLevelNodes{ nullptr };

	const TerrainTile *const m_terrainTile{nullptr};

};

}
