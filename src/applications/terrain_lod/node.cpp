
#include <applications/terrain_lod/heightmap.h>
#include <applications/terrain_lod/LODSelection.h>
#include <applications/terrain_lod/node.h>
#include <applications/terrain_lod/TerrainTile.h>
#include "base/logbook.h"
#include "geometry/aabb.h"
#include "geometry/view_frustum.h"
#include <sstream>

namespace terrain {

node::node() {}

void node::create( const int x, const int z, const int size, const int level,
		const TerrainTile *const terrainTile, node *allNodes, int &lastIndex ) {
	// @todo: plausibility checks: x and z between 0 and 65535; level between 0 and max lod levels,
	// size between 2 and 65535
	m_x = x;
	m_z = z;
	m_level = level;
	m_size = size;
	const heightmap *heightMap{ terrainTile->getHeightMap() };
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
	m_boundingBox = new orf_n::aabb{ min, max };
	// Highest level reached already ?
	if( size == LEAF_NODE_SIZE ) {
		if( level != NUMBER_OF_LOD_LEVELS -1 ) {
			std::string s{ "Lowest lod level unequals number lod levels during quad tree node creation." };
			orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::ERROR, s );
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

node::~node() {
	delete m_boundingBox;
}

const omath::vec2 &node::getMinMaxHeight() const {
	return m_minMaxHeight;
}

int node::getLevel() const {
	return m_level;
}

const orf_n::aabb *node::getBoundingBox() const {
	return m_boundingBox;
}

const node *node::getUpperRight() const {
	return m_subTR;
}

const node *node::getUpperLeft() const {
	return m_subTL;
}

const node *node::getLowerRight() const {
	return m_subBR;
}

const node *node::getLowerLeft() const {
	return m_subBL;
}

bool node::isLeaf() const {
	return m_isLeaf;
}

orf_n::intersect_t node::lodSelect( LODSelection *lodSelection, bool parentCompletelyInFrustum ) {
	// Shortcut
	const orf_n::camera *cam{ lodSelection->m_camera };
	// Test early outs
	orf_n::view_frustum f = cam->get_view_frustum();
	orf_n::intersect_t frustumIntersection = parentCompletelyInFrustum ?
			orf_n::INSIDE : cam->get_view_frustum().is_box_in_frustum( *m_boundingBox );
	if( orf_n::OUTSIDE == frustumIntersection )
		return orf_n::OUTSIDE;
	float distanceLimit = lodSelection->m_visibilityRanges[m_level];
	if( !m_boundingBox->intersect_sphere_sq( cam->get_position(), distanceLimit * distanceLimit ) )
		return orf_n::OUT_OF_RANGE;

	orf_n::intersect_t subTLSelRes = orf_n::UNDEFINED;
	orf_n::intersect_t subTRSelRes = orf_n::UNDEFINED;
	orf_n::intersect_t subBLSelRes = orf_n::UNDEFINED;
	orf_n::intersect_t subBRSelRes = orf_n::UNDEFINED;
	// Stop at one below number of lod levels
	if( m_level != lodSelection->m_stopAtLevel ) {
		float nextDistanceLimit = lodSelection->m_visibilityRanges[m_level+1];
		if( m_boundingBox->intersect_sphere_sq( cam->get_position(), nextDistanceLimit * nextDistanceLimit ) ) {
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
		orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::WARNING, s );
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
			lodSelection->m_selectedNodes[lodSelection->m_selectionCount].minDistanceTocamera =
					std::sqrt( lodSelection->m_selectedNodes[lodSelection->m_selectionCount].
					p_node->getBoundingBox()->min_distance_from_point_sq( cam->get_position() ) );
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
