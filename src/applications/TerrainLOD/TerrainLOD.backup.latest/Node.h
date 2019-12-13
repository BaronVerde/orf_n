
#pragma once

#include <applications/TerrainLOD/TerrainLOD.backup.latest/Settings.h>
#include <geometry/AABB.h>
#include <omath/vec2.h>
#include <memory>

namespace terrain {

class HeightMap;
class LODSelection;

class Node {
public:
	Node();

	virtual ~Node();

	bool isLeaf() const;

	/**
	 * Level 0 is a root node, and level 'LodLevel-1' is a leaf node. So the actual
	 * LOD level equals 'LODLevelCount - 1 - Node::GetLevel()'
	 */
	int getLevel() const;

	const orf_n::AABB *getBoundingBox() const;

	const omath::vec2 &getMinMaxHeight() const;

    void create( const int x, const int z, const int size, const int level, const orf_n::AABB *const wholeMapBB,
    		const terrain::HeightMap *heightMap, Node *allNodes, int &lastIndex );

    orf_n::intersect_t lodSelect( LODSelection *lodSelection, bool parentCompletelyInFrustum = false );

    const Node *getUpperRight() const;

    const Node *getUpperLeft() const;

    const Node *getLowerRight() const;

    const Node *getLowerLeft() const;

private:
	int m_x;

	int m_z;

	int m_size;

	bool m_isLeaf{ false };

	// @todo changed Caution: highest bit here is used to mark leaf nodes
	int m_level;

	omath::vec2 m_minMaxHeight;

    // When isLeaf() these can be reused for something else.
	// currently they store float heights for ray triangle test but that could be
	// stored in a matrix without 4x redundancy like here. Also, these could/should be
	// indices into CDLODQuadTree::m_allNodesBuffer - no additional memory will then be used
    // if compiled for 64bit, and they could also be unsigned short-s if having less 65535 nodes
    Node *m_subTL{ nullptr };
    Node *m_subTR{ nullptr };
    Node *m_subBL{ nullptr };
    Node *m_subBR{ nullptr };

    const orf_n::AABB *m_boundingBox{ nullptr };

};

}
