
#include <applications/TerrainLOD/zBackup_SinglePrecision/LODSelection.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/Node.h>
#include <applications/TerrainLOD/zBackup_SinglePrecision/QuadTree.h>
#include <base/Logbook.h>
#include <omath/common.h>	// lerp()
#include <sstream>
#include <iostream>

namespace terrain {

LODSelection::LODSelection( const orf_n::Camera *cam, bool sortByDistance ) :
		m_camera{ cam }, m_sortByDistance{ sortByDistance } {
	calculateRanges();
}

LODSelection::~LODSelection() {}

void LODSelection::calculateRanges() {
	float total{ 0 };
	float currentDetailBalance{ 1.0f };
	for( int i{ 0 }; i < NUMBER_OF_LOD_LEVELS; ++i ) {
		total += currentDetailBalance;
		currentDetailBalance *= LOD_LEVEL_DISTANCE_RATIO;
	}
	float sect{ ( m_camera->getFarPlane() - m_camera->getNearPlane() ) / total };
	float prevPos{ m_camera->getNearPlane() };
	currentDetailBalance = 1.0f;
	for( int i{ 0 }; i < NUMBER_OF_LOD_LEVELS; ++i ) {
		// @todo why is this inverted ?
		m_visibilityRanges[NUMBER_OF_LOD_LEVELS - i - 1] = prevPos + sect * currentDetailBalance;
		prevPos = m_visibilityRanges[NUMBER_OF_LOD_LEVELS - i - 1];
		currentDetailBalance *= LOD_LEVEL_DISTANCE_RATIO;
	}
	prevPos = m_camera->getNearPlane();
	std::ostringstream s;
	s << "Lod levels and ranges: lvl/range/start/end ";
	for( int i{ 0 }; i < NUMBER_OF_LOD_LEVELS; ++i ) {
		int index{ NUMBER_OF_LOD_LEVELS - i - 1 };
		m_morphEnd[i] = m_visibilityRanges[index];
		m_morphStart[i] = prevPos + ( m_morphEnd[i] - prevPos ) * MORPH_START_RATIO;
		prevPos = m_morphStart[i];
		// Debug output:
		//s << i << '/' << m_visibilityRanges[NUMBER_OF_LOD_LEVELS - i - 1] << '/' << m_morphStart[i] << '/' << m_morphEnd[i] << "; ";
	}
	//orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, s.str() );
}

void LODSelection::reset() {
	m_selectionCount = 0;
	m_isVisibilityDistanceTooSmall = false;
	m_maxSelectedLODLevel = 0;
	m_minSelectedLODLevel = NUMBER_OF_LOD_LEVELS;
}

static inline int compareCloserFirst( const void *arg1, const void *arg2 ) {
	const LODSelection::selectedNode_t *a = (const LODSelection::selectedNode_t *)arg1;
	const LODSelection::selectedNode_t *b = (const LODSelection::selectedNode_t *)arg2;
	return a->minDistanceToCamera > b->minDistanceToCamera;
}

// sort by tile index and distance
void LODSelection::setDistancesAndSort() {
	if( m_sortByDistance )
		std::qsort( m_selectedNodes, m_selectionCount, sizeof( *m_selectedNodes ), compareCloserFirst );
	// Debug output:
	/*std::ostringstream s;
	s << "New selection :\n";
	for( int i{ 0 }; i < m_selectionCount; ++i ) {
		selectedNode_t n{ m_selectedNodes[i] };
		s << "Node " << *n.node->getBoundingBox() << "; tile " << n.tileIndex << "; lvl " << n.lodLevel <<
				"; distance " << n.minDistanceToCamera << '\n';
	}
	std::cout << s.str();*/
}

const omath::vec4 LODSelection::getMorphConsts( const int lodLevel ) const {
	const float mStart{ m_morphStart[lodLevel] };
	float mEnd{ m_morphEnd[lodLevel] };
	const float errorFudge{ 0.01f };
	mEnd = omath::lerp( mEnd, mStart, errorFudge );
	const float d{ mEnd - mStart };
	return omath::vec4{ mStart, 1.0f / d, mEnd / d, 1.0f / d };
}

}
