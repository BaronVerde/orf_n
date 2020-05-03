
#pragma once

#include "omath/vec3.h"

namespace terrain {

class cascaded_volume_map {
public:

	enum SettingsFlags {
		SF_IgnoreZ = (1 << 0),
	};

	struct settings {
		// create bigger volume to accommodate some amount of movement before data needs to be recreated
		float volume_extension_mul{1.15f};
		// how much of extended space to use (defined by VolumeExtensionMul) to accommodate
		// for the predicted future observer movement when recalculating the volume
		float movement_prediction_offset_mul{0.5f};
		// how many layers can we update during one frame
		float allowed_updates_per_frame{2.0f};
		float fade_in_time{0.5f};
		unsigned int flags{0};
	};

	// single cascade layer
	struct layer {
		omath::vec3 box_min{0.0f};
		omath::vec3 box_max{0.0f};
		omath::vec3 prev_change_obs_pos{0.0f};
		omath::vec3 avg_obs_movement_dir{0.0f};
		virtual bool update( const omath::vec3& _observer_pos, float new_visibility_range,
				cascaded_volume_map* parent, bool force_update = false );
	};

protected:
	settings m_settings;
	layer** m_layers_array{nullptr};
	int m_layer_count{0};
	double m_currentTime{0.0f};
	float m_update_frame_budget{1.0f};
	int m_lowest_updated{0xFF};

	cascaded_volume_map();

	virtual ~cascaded_volume_map();

	// Resets all layers to 0 bounding boxes
	virtual void reset();

	virtual int update( const omath::vec3& observer_pos, const float visibility_ranges[] );

};

}	// namespace
