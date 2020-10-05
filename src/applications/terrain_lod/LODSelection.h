
/**
 * Selects visible nodes and lod levels from a quad tree based on
 * lod settings and camera frustum.
 */

#pragma once

#include <applications/terrain_lod/settings.h>
#include "applications/camera/camera.h"
#include "omath/vec4.h"

namespace terrain {

class node;
class quad_tree;

class LODSelection {
public:
	typedef struct selectedNode_t {
		node *p_node{ nullptr };
		int tileIndex{ -1 };
		int lodLevel{ -1 };
		bool hasTL{ false };
		bool hasTR{ false };
		bool hasBL{ false };
		bool hasBR{ false };
		double minDistanceTocamera{ 0.0 };	// for sorting by distance

		selectedNode_t() {};

		selectedNode_t( node *n, int tileIndex, int lvl, bool tl, bool tr, bool bl, bool br ) :
			p_node{n}, tileIndex{tileIndex}, lodLevel{lvl}, hasTL{tl}, hasTR{tr}, hasBL{bl}, hasBR{br} {}
	} selectedNode_t;

	LODSelection( const orf_n::camera *cam, bool sortByDistance = false );

	virtual ~LODSelection();

	/**
	 * Called when camera near or far plane changed to recalc visibility
	 * and morph ranges.
	 */
	void calculateRanges();

	void setDistancesAndSort();

	void reset();

	void print_selection() const;

	const omath::vec4 getMorphConsts( const int lodLevel ) const;

	const orf_n::camera *m_camera{ nullptr };

	float m_visibilityRanges[NUMBER_OF_LOD_LEVELS];

	bool m_sortByDistance{ false };

	float m_morphStart[NUMBER_OF_LOD_LEVELS];

	float m_morphEnd[NUMBER_OF_LOD_LEVELS];

	const int m_stopAtLevel{ NUMBER_OF_LOD_LEVELS };

	int m_currentTileIndex{ -1 };

	int m_selectionCount{ 0 };

	selectedNode_t m_selectedNodes[MAX_NUMBER_SELECTED_NODES];

	int m_maxSelectedLODLevel{ 0 };

	int m_minSelectedLODLevel{ NUMBER_OF_LOD_LEVELS };

};

}
