
#pragma once

#include "settings.h"
#include "geometry/aabb.h"
#include "omath/vec2.h"
#include <memory>

namespace terrain {

class heightmap;
class LODSelection;
class TerrainTile;

class node {
public:
	node();

	virtual ~node();

	bool isLeaf() const;

	/**
	 * Level 0 is a root node, and level 'LodLevel-1' is a leaf node. So the actual
	 * LOD level equals 'LODLevelCount - 1 - Node::GetLevel()'
	 */
	int getLevel() const;

	const orf_n::aabb *getBoundingBox() const;

	const omath::vec2 &getMinMaxHeight() const;

	// worldPositionCellsize: .x = lower left latitude, .y = longitude, .z = cellsize
    void create( const int x, const int z, const int size, const int level,
    		const TerrainTile *const terrainTile, node *allNodes, int &lastIndex );

    orf_n::intersect_t lodSelect( LODSelection *lodSelection, bool parentCompletelyInFrustum = false );

    const node *getUpperRight() const;

    const node *getUpperLeft() const;

    const node *getLowerRight() const;

    const node *getLowerLeft() const;

private:
	int m_x;

	int m_z;

	int m_size;

	bool m_isLeaf{ false };

	// @todo changed Caution: highest bit here is used to mark leaf nodes
	int m_level;

	omath::vec2 m_minMaxHeight;

    // @todo When isLeaf() these can be reused for something else.
	// currently they store float heights for ray triangle test but that could be
	// stored in a matrix without 4x redundancy like here. Also, these could/should be
	// indices into CDLODQuadTree::m_allNodesBuffer - no additional memory will then be used
    // if compiled for 64bit, and they could also be unsigned short-s if having less 65535 nodes
    node *m_subTL{ nullptr };
    node *m_subTR{ nullptr };
    node *m_subBL{ nullptr };
    node *m_subBR{ nullptr };

    const orf_n::aabb* m_boundingBox{ nullptr };

};

}
