
#pragma once

#include "omath/vec3.h"

namespace terrain {

class cascaded_volume_map {
public:

	enum SettingsFlags {
		SF_IgnoreZ = (1 << 0),
	};

	struct Settings {
		// create bigger volume to accommodate some amount of movement before data needs to be recreated
		float VolumeExtensionMul{1.15f};
		// how much of extended space to use (defined by VolumeExtensionMul) to accommodate
		// for the predicted future observer movement when recalculating the volume
		float MovementPredictionOffsetMul{0.5f};
		// how many layers can we update during one frame
		float AllowedUpdatesPerFrame{2.0f};
		float FadeInTime{0.5f};
		unsigned int Flags{0};
	};

	// single cascade layer
	struct Layer {
		omath::vec3 BoxMin{0.0f};
		omath::vec3 BoxMax{0.0f};
		omath::vec3 PrevChangeObsPos{0.0f};
		omath::vec3 AvgObsMovementDir{0.0f};
		virtual bool Update( const omath::vec3& _observerPos, float newVisibilityRange,
				cascaded_volume_map* parent, bool forceUpdate = false );
	};

protected:
	Settings m_settings;
	Layer** m_layersArray{nullptr};
	int m_layerCount{0};
	double m_currentTime{0.0f};
	float m_updateFrameBudget{1.0f};
	int m_lowestUpdated{0xFF};

	cascaded_volume_map();

	virtual ~cascaded_volume_map();

	// Resets all layers to 0 bounding boxes
	virtual void Reset();

	virtual int Update( float deltaTime, const omath::vec3& observerPos, const float visibilityRanges[] );

};

}	// namespace
