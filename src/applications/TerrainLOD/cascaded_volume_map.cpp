
#include "cascaded_volume_map.h"
#include <cassert>

namespace terrain {

cascaded_volume_map::cascaded_volume_map() {}

cascaded_volume_map::~cascaded_volume_map() {
	assert( m_layersArray == nullptr );
}

// single cascade layer
bool cascaded_volume_map::Layer::Update( const omath::vec3& _observerPos, float newVisibilityRange,
		cascaded_volume_map* parent, bool forceUpdate ) {
	omath::vec3 observerPos = _observerPos;
	const Settings & settings = parent->m_settings;
	omath::vec3 halfRange = omath::vec3{ newVisibilityRange };
	omath::vec3 newBoxMin = observerPos - halfRange;
	omath::vec3 newBoxMax = observerPos + halfRange;
	// if new box is inside current one, everything is fine then, return false
	if( !forceUpdate
			&& (newBoxMin.x >= BoxMin.x) && (newBoxMax.x <= BoxMax.x)
			&& (newBoxMin.y >= BoxMin.y) && (newBoxMax.y <= BoxMax.y)
			&& ( ((newBoxMin.z >= BoxMin.z) && (newBoxMax.z <= BoxMax.z)) || (settings.Flags & SF_IgnoreZ) != 0 ) )
		return false;
	AvgObsMovementDir = AvgObsMovementDir + (observerPos - PrevChangeObsPos);
	AvgObsMovementDir = omath::normalize( AvgObsMovementDir );
	PrevChangeObsPos = observerPos;
	observerPos += ( (settings.VolumeExtensionMul - 1.0f) * newVisibilityRange *
			settings.MovementPredictionOffsetMul ) * AvgObsMovementDir;
	float a = newVisibilityRange * settings.VolumeExtensionMul;
	halfRange = omath::vec3{a};
	BoxMin = observerPos - halfRange;
	BoxMax = observerPos + halfRange;
	return true;
}

// Resets all layers to 0 bounding boxes
void cascaded_volume_map::Reset() {
	for( int i = m_layerCount-1; i >= 0 && m_updateFrameBudget >= 1.0f; i-- ) {
		Layer * layer = m_layersArray[i];
		layer->BoxMin += layer->BoxMax;
		layer->BoxMin *= 0.5;
		layer->BoxMax = layer->BoxMin;
	}
}

int cascaded_volume_map::Update(
		float deltaTime, const omath::vec3& observerPos, const float visibilityRanges[] ) {
	m_currentTime += deltaTime;
	m_updateFrameBudget += m_settings.AllowedUpdatesPerFrame;
	if( m_updateFrameBudget > m_settings.AllowedUpdatesPerFrame )
		m_updateFrameBudget = m_settings.AllowedUpdatesPerFrame;
#if _DEBUG
	for( int i = 0; i < m_layerCount-1; i++ )
		assert( visibilityRanges[i] < visibilityRanges[i+1] );
	assert( m_settings.VolumeExtensionMul >= 1.0f );
	assert( m_settings.MovementPredictionOffsetMul >= 0.0f && m_settings.MovementPredictionOffsetMul < 1.0f );
	assert( m_settings.AllowedUpdatesPerFrame > 0.0f );
	assert( m_settings.FadeInTime > 0.0f );
#endif
	int numberUpdated = 0;
	// Update layers if needed
	for( int i = m_layerCount-1; i >= 0 && m_updateFrameBudget >= 1.0f; i-- ) {
		Layer* layer = m_layersArray[i];
		if( layer->Update( observerPos, visibilityRanges[i], this ) ) {
			numberUpdated ++;
			m_updateFrameBudget -= 1.0f;
		}
		m_lowestUpdated = i;
	}
	return numberUpdated;
}

} /* namespace terrain */
