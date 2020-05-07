
#include "cascaded_volume_map.h"
#include "base/globals.h"
#include "settings.h"
#include <cassert>

namespace terrain {

cascaded_volume_map::cascaded_volume_map() {}

cascaded_volume_map::~cascaded_volume_map() {
	assert( m_layers_array == nullptr );
}

// single cascade layer
bool cascaded_volume_map::layer::update( const omath::vec3& observer_pos, float new_visibility_range,
		cascaded_volume_map* parent, bool force_update ) {
	omath::vec3 new_observer_pos = observer_pos;
	const settings& settings = parent->m_settings;
	omath::vec3 half_range = omath::vec3{new_visibility_range};
	omath::vec3 new_box_min = new_observer_pos - half_range;
	omath::vec3 new_box_max = new_observer_pos + half_range;
	// if new box is inside current one, everything is fine then, return false
	if( !force_update &&
		(new_box_min.x >= box_min.x) && (new_box_max.x <= box_max.x) &&
		(new_box_min.y >= box_min.y) && (new_box_max.y <= box_max.y) &&
		( ((new_box_min.z >= box_min.z) && (new_box_max.z <= box_max.z)) || (settings.flags & SF_IgnoreZ) != 0 ) )
		return false;
	avg_obs_movement_dir = avg_obs_movement_dir + (new_observer_pos - prev_change_obs_pos);
	avg_obs_movement_dir = omath::normalize( avg_obs_movement_dir );
	prev_change_obs_pos = new_observer_pos;
	new_observer_pos += ( (settings.volume_extension_mul - 1.0f) * new_visibility_range *
			settings.movement_prediction_offset_mul ) * avg_obs_movement_dir;
	float a = new_visibility_range * settings.volume_extension_mul;
	half_range = omath::vec3{a};
	box_min = new_observer_pos - half_range;
	box_max = new_observer_pos + half_range;
	return true;
}

// Resets all layers to 0 bounding boxes
void cascaded_volume_map::reset() {
	for( int i = NUMBER_OF_LOD_LEVELS-1; i >= 0 && m_update_frame_budget >= 1.0f; --i ) {
		layer* layer = m_layers_array[i];
		layer->box_min += layer->box_max;
		layer->box_min *= 0.5;
		layer->box_max = layer->box_min;
	}
}

int cascaded_volume_map::update( const omath::vec3& observer_pos, const float visibility_ranges[] ) {
	m_currentTime += orf_n::globals::delta_time;
	m_update_frame_budget += m_settings.allowed_updates_per_frame;
	if( m_update_frame_budget > m_settings.allowed_updates_per_frame )
		m_update_frame_budget = m_settings.allowed_updates_per_frame;
#if _DEBUG
	for( int i = 0; i < NUMBER_OF_LOD_LEVELS-1; i++ )
		assert( visibility_ranges[i] < visibility_ranges[i+1] );
	assert( m_settings.volume_extension_mul >= 1.0f );
	assert( m_settings.movement_prediction_offset_mul >= 0.0f && m_settings.movement_prediction_offset_mul < 1.0f );
	assert( m_settings.allowed_updates_per_frame > 0.0f );
	assert( m_settings.fade_in_time > 0.0f );
#endif
	int number_updated = 0;
	// Update layers if needed
	for( int i = NUMBER_OF_LOD_LEVELS-1; i >= 0 && m_update_frame_budget >= 1.0f; i-- ) {
		layer* layer = m_layers_array[i];
		if( layer->update( observer_pos, visibility_ranges[i], this ) ) {
			number_updated++;
			m_update_frame_budget -= 1.0f;
		}
		m_lowest_updated = i;
	}
	return number_updated;
}

} /* namespace terrain */
