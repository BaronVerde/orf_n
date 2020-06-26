
#pragma once

#include "omath/vec2.h"
#include "scene/renderable.h"
//#include <random>

// general @todo: set boundary flows to 0 !
// extend model to the eight neighbour cells !

class HydroErosionMDH07 : public orf_n::renderable {
public:

	//static std::minstd_rand randomEngine(42);
	// [0.0, 1.0)
	//static std::uniform_real_distribution<float> rGen( 0.0f, 1.0f );

	HydroErosionMDH07();

	virtual ~HydroErosionMDH07();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

	void resetWaterTable( const float globalLevel );

	// Increase due to rain or river flow
	void incrementWater( const float deltatime );

	void calculateFlux( const float deltatime );

	void updateWaterheight( const float deltatime );

	void calculateVelocityField( const float deltatime );

	void calculateErosionDeposition();

	// Transport sediment with velocity field
	void transportSediment( const float deltatime );

	void evaporateWater( const float deltatime );

private:

	struct {
		const uint16_t dimension{8};
		const float gravity{9.81f};
		// @todo: amount of water per cell per turn. This must be variable, e.g. from a texture
		const float aDropOfRain{0.1f};
		const float rainMultiplier{1.0f};
		// length as well for now
		const float cellsize{1.0f};
		const float pipeCrossSection{1.0f};
		const float pipeLength{cellsize};
		// distances between grid points
		const float distanceX_LX{cellsize};
		const float distanceY_LY{cellsize};
		const float sedimentCapacityConstant_KC{1.0f};
		// To avoid transport capacity going to 0 in flat terrain
		const float minFlowAngle{0.001f};
		const float dissolvingConstant_KS{1.0f};
		const float depositionConstant_KD{1.0f};
		const float evaporationConstant_KE{1.0f};
	} SETTINGS;

	typedef enum { left=0, right, top, bottom } direction_t;

	typedef struct cell_t {
		float terrainHeight_b{0.0f};
		float waterHeight_d2{0.0f};
		float tempWaterHeight_d1{0.0f};
		float tempSuspendedSediment_st{0.0f};
		float suspendedSedimentThisTurn_s1{0.0f};
		float sudpendedSediment_s{0.0f};
		float sedimentTransportCapacity_C{0.0f};
		// outflow in direction 0..3
		float flux_f[4]{0.0f,0.0f,0.0f,0.0f};
		omath::vec2 velocity_v{0.0f};
		float getTerrainHeightDiff( const cell_t &other ) const {
			return terrainHeight_b - other.terrainHeight_b;
		}
		float getTotalHeightDiff( const cell_t &other ) const {
			return terrainHeight_b + tempWaterHeight_d1 -
				   other.terrainHeight_b - other.tempWaterHeight_d1;
		}
	} cell_t;

	cell_t **grid{nullptr};

	// @todo velocity field as own grid ?

	void outputHeight();

	void outputTempWater();

	void outputFlux();

	void outputVelocity();

};
