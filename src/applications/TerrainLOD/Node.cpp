
#include "HeightMap.h"
#include "LODSelection.h"
#include "Node.h"
#include "TerrainTile.h"
#include "base/Logbook.h"
#include "geometry/AABB.h"
#include "geometry/ViewFrustum.h"
#include <sstream>

namespace terrain {

Node::Node() {}

void Node::create( const int x, const int z, const int size, const int level,
		const TerrainTile *const terrainTile, Node *allNodes, int &lastIndex ) {
	// @todo: plausibility checks: x and z between 0 and 65535; level between 0 and max lod levels,
	// size between 2 and 65535
	m_x = x;
	m_z = z;
	m_level = level;
	m_size = size;
	const HeightMap *heightMap{ terrainTile->getHeightMap() };
	// Find min/max heights at this patch of terrain
	const int limitX = std::min( heightMap->getExtent().x, x + size + 1 );
	// limit z = y-axis of heightmap
	const int limitZ = std::min( heightMap->getExtent().y, z + size + 1 );
	// @todo: check height read out of heightmap positions (int) and coordinates (float)
	m_minMaxHeight = heightMap->getMinMaxHeightArea( x, z, limitX - x, limitZ - z );
	// Get bounding box in world coords
	const omath::dvec3 min{
		terrainTile->getAABB()->m_min.x + m_x, m_minMaxHeight.x, terrainTile->getAABB()->m_min.z + m_z
	};
	const omath::dvec3 max{
		terrainTile->getAABB()->m_min.x + m_x + m_size, m_minMaxHeight.y, terrainTile->getAABB()->m_min.z + m_z + m_size
	};
	m_boundingBox = new orf_n::AABB{ min, max };
	// Highest level reached already ?
	if( size == LEAF_NODE_SIZE ) {
		if( level != NUMBER_OF_LOD_LEVELS -1 ) {
			std::string s{ "Lowest lod level unequals number lod levels during quad tree node creation." };
			orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::ERROR, s );
			throw std::runtime_error( s );
		}
		// Mark leaf node!
	    m_isLeaf = true;
	} else {
		int subSize = size / 2;
		m_subTL = &allNodes[lastIndex++];
		m_subTL->create( x, z, subSize, level+1, terrainTile, allNodes, lastIndex );
		if( ( x + subSize ) < heightMap->getExtent().x ) {
	         m_subTR = &allNodes[lastIndex++];
	         m_subTR->create( x + subSize, z, subSize, level+1, terrainTile, allNodes, lastIndex );
		}
		if( (z + subSize) < heightMap->getExtent().y ) {
			m_subBL = &allNodes[lastIndex++];
			m_subBL->create( x, z + subSize, subSize, level+1, terrainTile, allNodes, lastIndex );
		}
		if( ( ( x + subSize ) < heightMap->getExtent().x ) && ( ( z + subSize ) < heightMap->getExtent().y ) ) {
			m_subBR = &allNodes[lastIndex++];
			m_subBR->create( x + subSize, z + subSize, subSize, level+1, terrainTile, allNodes, lastIndex );
		}
	}
}

Node::~Node() {
	delete m_boundingBox;
}

const omath::vec2 &Node::getMinMaxHeight() const {
	return m_minMaxHeight;
}

int Node::getLevel() const {
	return m_level;
}

const orf_n::AABB *Node::getBoundingBox() const {
	return m_boundingBox;
}

const Node *Node::getUpperRight() const {
	return m_subTR;
}

const Node *Node::getUpperLeft() const {
	return m_subTL;
}

const Node *Node::getLowerRight() const {
	return m_subBR;
}

const Node *Node::getLowerLeft() const {
	return m_subBL;
}

bool Node::isLeaf() const {
	return m_isLeaf;
}

orf_n::intersect_t Node::lodSelect( LODSelection *lodSelection, bool parentCompletelyInFrustum ) {
	// Shortcut
	const orf_n::Camera *cam{ lodSelection->m_camera };
	// Test early outs
	orf_n::intersect_t frustumIntersection = parentCompletelyInFrustum ?
			orf_n::INSIDE : cam->getViewFrustum().isBoxInFrustum( m_boundingBox );
	if( orf_n::OUTSIDE == frustumIntersection )
		return orf_n::OUTSIDE;
	float distanceLimit = lodSelection->m_visibilityRanges[m_level];
	if( !m_boundingBox->intersectSphereSq( cam->getPosition(), distanceLimit * distanceLimit ) )
		return orf_n::OUT_OF_RANGE;

	orf_n::intersect_t subTLSelRes = orf_n::UNDEFINED;
	orf_n::intersect_t subTRSelRes = orf_n::UNDEFINED;
	orf_n::intersect_t subBLSelRes = orf_n::UNDEFINED;
	orf_n::intersect_t subBRSelRes = orf_n::UNDEFINED;
	// Stop at one below number of lod levels
	if( m_level != lodSelection->m_stopAtLevel ) {
		float nextDistanceLimit = lodSelection->m_visibilityRanges[m_level+1];
		if( m_boundingBox->intersectSphereSq( cam->getPosition(), nextDistanceLimit * nextDistanceLimit ) ) {
			bool weAreCompletelyInFrustum = frustumIntersection == orf_n::INSIDE;
			if( m_subTL != nullptr )
				subTLSelRes = m_subTL->lodSelect( lodSelection, weAreCompletelyInFrustum );
			if( m_subTR != nullptr )
				subTRSelRes = m_subTR->lodSelect( lodSelection, weAreCompletelyInFrustum );
			if( m_subBL != nullptr )
				subBLSelRes = m_subBL->lodSelect( lodSelection, weAreCompletelyInFrustum );
			if( m_subBR != nullptr )
				subBRSelRes = m_subBR->lodSelect( lodSelection, weAreCompletelyInFrustum );
		}
	}

	// We don't want to select sub nodes that are invisible (out of frustum) or are selected;
	// (we DO want to select if they are out of range, since we are not)
	bool removeSubTL = (subTLSelRes == orf_n::OUTSIDE) || (subTLSelRes == orf_n::SELECTED);
	bool removeSubTR = (subTRSelRes == orf_n::OUTSIDE) || (subTRSelRes == orf_n::SELECTED);
	bool removeSubBL = (subBLSelRes == orf_n::OUTSIDE) || (subBLSelRes == orf_n::SELECTED);
	bool removeSubBR = (subBRSelRes == orf_n::OUTSIDE) || (subBRSelRes == orf_n::SELECTED);

	if( lodSelection->m_selectionCount >= MAX_NUMBER_SELECTED_NODES ) {
		std::string s{ "LOD selected more nodes than the maximum selection count. Some nodes will not be drawn." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
		return orf_n::OUTSIDE;
	}
	// Add node to selection
	if( !( removeSubTL && removeSubTR && removeSubBL && removeSubBR ) &&
		 ( lodSelection->m_selectionCount < MAX_NUMBER_SELECTED_NODES ) ) {
		int lodLevel = lodSelection->m_stopAtLevel - m_level;
		lodSelection->m_selectedNodes[lodSelection->m_selectionCount] =
				LODSelection::selectedNode_t( this, lodSelection->m_currentTileIndex, lodLevel,
						!removeSubTL, !removeSubTR, !removeSubBL, !removeSubBR );
		lodSelection->m_minSelectedLODLevel = std::min( lodSelection->m_minSelectedLODLevel,
				lodSelection->m_selectedNodes[lodSelection->m_selectionCount].lodLevel );
		lodSelection->m_maxSelectedLODLevel = std::max( lodSelection->m_maxSelectedLODLevel,
				lodSelection->m_selectedNodes[lodSelection->m_selectionCount].lodLevel );
		// Set tile index, min distance and min/max levels for sorting
		// @todo sorting temporarily disabled
		if( lodSelection->m_sortByDistance )
			lodSelection->m_selectedNodes[lodSelection->m_selectionCount].minDistanceToCamera =
					std::sqrt( lodSelection->m_selectedNodes[lodSelection->m_selectionCount].
					node->getBoundingBox()->minDistanceFromPointSq( cam->getPosition() ) );
		lodSelection->m_selectionCount++;
		return orf_n::SELECTED;
	}
	// if any of child nodes are selected, then return selected -
	// otherwise all of them are out of frustum, so we're out of frustum too
	if( (subTLSelRes == orf_n::SELECTED) || (subTRSelRes == orf_n::SELECTED) ||
		(subBLSelRes == orf_n::SELECTED) || (subBRSelRes == orf_n::SELECTED) )
		return orf_n::SELECTED;
	else
		return orf_n::OUTSIDE;
}

}

/*
 * 	    // Find heights for 4 corner points (used for approx ray casting)
	    // (reuse otherwise empty pointers used for sub nodes)
	    float * pTLZ = (float *)&subTL;
	    float * pTRZ = (float *)&subTR;
	    float * pBLZ = (float *)&subBL;
	    float * pBRZ = (float *)&subBR;
	    int limitX = ::min( rasterSizeX - 1, x + size );
	    int limitY = ::min( rasterSizeY - 1, y + size );
	    *pTLZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( x, y ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pTRZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( limitX, y ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pBLZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( x, limitY ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pBRZ = createDesc.MapDims.MinZ + createDesc.pHeightmap->GetHeightAt( limitX, limitY ) * createDesc.MapDims.SizeZ / 65535.0f;
 */
