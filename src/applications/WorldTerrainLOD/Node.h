
#pragma once

#include "Settings.h"
#include "geometry/AABB.h"
#include "geometry/Ellipsoid.h"
#include "geometry/Geodetic.h"
#include "omath/vec3.h"
#include <memory>

namespace terrain {

class HeightMap;
class LODSelection;
class TerrainTile;

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

	const orf_n::AABB *getWorldBoundingBox() const;

	/**
	 * Bounding box relative to tile. x and z of min and max are 2d coords,
	 * y holds min and max height.
	 */
	const orf_n::AABB *getTileBoundingBox() const;

	const omath::vec3 &getGeodeticSurfaceNormal() const;

	const omath::vec2 &getMinMaxHeight() const;

	const omath::ivec2 getXZ() const;

	/**
	 * x/y/size are relative to the heightmap, 0/0 being the lower left, size of the node in posts
	 * level is the od level @todo which is which ?
	 * terrainTile a pointer to the terrain tile to look up necessary information
	 * In/out: pointer to list of nodes nodes and last used index
	 */
    void create( const int x, const int z, const int size, const int level,
    		const TerrainTile *const terrainTile, Node *allNodes, int &lastIndex );

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

    /**
     * BB in world coordinates for frustum check and world position calculation.
     * These are used to select inj world space what is being drawn.
     */
    const orf_n::AABB *m_worldBoundingBox{ nullptr };

    /**
     * BB in tile coordinates, to calculate texture positions in the shader.
     * These are passed to the vertex shader to calculate height posts on the heightmap
     * via offset and scale of the current node.
     */
    const orf_n::AABB *m_tileBoundingBox{nullptr};

    /**
     * Geodetic surface normal of the node (the lower left coordinate).
     * For now, this is calculated for the whole node and passed into the shader for performance
     * and to avoid double precision multiplications in there.
     */
    omath::vec3 m_geodeticSurfaceNormal;

};

}
