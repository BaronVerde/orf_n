
#pragma once

#include "omath/vec3.h"
#include <array>
#include <random>
#include "scene/Renderable.h"

// general @todo: set boundary flows to 0 !
// extend model to the eight neighbour cells !

class HydroErosionMDH07 : public orf_n::Renderable {
public:

	//static std::minstd_rand randomEngine(42);
	// [0.0, 1.0)
	//static std::uniform_real_distribution<float> rGen( 0.0f, 1.0f );

	static struct {
		const uint16_t dimension{4};
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

	HydroErosionMDH07() : orf_n::Renderable( "HydroErosionMDH07" ) {}

	virtual ~HydroErosionMDH07() {}

	virtual void setup() override final {
		grid = new cell_t *[SETTINGS.dimension];
		for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
			grid[i] = new cell_t[SETTINGS.dimension];
	}

	virtual void cleanup() override final {
		for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
			delete [] grid[i];
		delete [] grid;
	}

typedef enum { left=0, right, top, bottom } direction_t;

typedef struct cell_t {
	float terrainHeight_b;
	float waterHeight_d2;
	float tempWaterHeight_d1;
	float tempSuspendedSediment_st;
	float suspendedSedimentThisTurn_s1;
	float sudpendedSediment_s;
	float sedimentTransportCapacity_C;
	// outflow in direction 0..3
	float flux_f[4];
	omath::vec2 velocity_v;

	float getTerrainHeightDiff( const cell_t &other ) const {
		return terrainHeight_b - other.terrainHeight_b;
	}

	float getTotalHeightDiff( const cell_t &other ) const {
		return terrainHeight_b + tempWaterHeight_d1 -
			   other.terrainHeight_b - other.tempWaterHeight_d1;
	}

};

// Increase due to rain or river flow
void incrementWater( const float deltatime ) {
	// @todo: this can be result of some more complex process
	float waterThisTurn_r{ SETTINGS.aDropOfRain * SETTINGS.rainMultiplier };
	for( uint16_t i{0}; i <= SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j <= SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			c.tempWaterHeight_d1 = c.waterHeight_d2 + deltatime * waterThisTurn_r;
		}
}

void calculateFlux( const float deltatime ) {
	for( uint16_t i{0}; i <= SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j <= SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			c.flux_f[left] = std::max( 0.0f, c.flux_f[left] ) + deltatime * SETTINGS.pipeCrossSection *
					( ( SETTINGS.gravity * c.getTotalHeightDiff( grid[i-1][j] ) ) / SETTINGS.pipeLength );
			c.flux_f[right] = std::max( 0.0f, c.flux_f[right] ) + deltatime * SETTINGS.pipeCrossSection *
					( ( SETTINGS.gravity * c.getTotalHeightDiff( grid[i+1][j] ) ) / SETTINGS.pipeLength );
			c.flux_f[top] = std::max( 0.0f, c.flux_f[top] ) + deltatime * SETTINGS.pipeCrossSection *
					( ( SETTINGS.gravity * c.getTotalHeightDiff( grid[i][j-1] ) ) / SETTINGS.pipeLength );
			c.flux_f[bottom] = std::max( 0.0f, c.flux_f[bottom] ) + deltatime * SETTINGS.pipeCrossSection *
					( ( SETTINGS.gravity * c.getTotalHeightDiff( grid[i][j+1] ) ) / SETTINGS.pipeLength );
			const float scalingFactor_K{
				std::min( 1.0f, ( c.tempWaterHeight_d1 * SETTINGS.pipeCrossSection ) /
				( deltatime * ( c.flux_f[left] + c.flux_f[right] + c.flux_f[top] + c.flux_f[bottom] ) ) )
			};
			c.flux_f[left] *= scalingFactor_K;
			c.flux_f[right] *= scalingFactor_K;
			c.flux_f[top] *= scalingFactor_K;
			c.flux_f[bottom] *= scalingFactor_K;
		}
}

void updateWaterheight( const float deltatime ) {
	for( uint16_t i{0}; i <= SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j <= SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			const float volumeChange{ deltatime *
				( grid[i-1][j].flux_f[right] + grid[i+1][j].flux_f[left] +
				  grid[i][j-1].flux_f[bottom] + grid[i][j+1].flux_f[top ] ) -
				( c.flux_f[left] + c.flux_f[right] + c.flux_f[top] + c.flux_f[bottom] ) };
			c.waterHeight_d2 = c.tempWaterHeight_d1 + ( volumeChange / SETTINGS.pipeCrossSection );
		};
}

void calculateVelocityField( const float deltatime ) {
	for( uint16_t i{0}; i <= SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j <= SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			float averagePassingWater_WX{
				( grid[i-1][j].flux_f[right] - c.flux_f[left] + grid[i+1][j].flux_f[left] - c.flux_f[right] ) / 2.0f
			};
			float averagePassingWater_WY{
				( grid[i-1][j].flux_f[right] - c.flux_f[left] + grid[i+1][j].flux_f[left] - c.flux_f[right] ) / 2.0f
			};
			// @todo Should be waterheigt of last two turns. Eventually extra pass.
			float meanWaterHeight{ ( c.tempWaterHeight_d1 + c.waterHeight_d2 ) / 2.0f };
			c.velocity_v.x = averagePassingWater_WX / ( SETTINGS.distanceX_LX * meanWaterHeight );
			c.velocity_v.y = averagePassingWater_WY / ( SETTINGS.distanceY_LY * meanWaterHeight );
			// @todo: Stability condition: adjust time step to smaller values for smaller distances in grid
			if( deltatime * averagePassingWater_WX > SETTINGS.distanceX_LX ||
				deltatime * averagePassingWater_WY > SETTINGS.distanceY_LY )
				std::cout << "Time step too long, simulation unstable\n";
		}
}

void calculateErosionDeposition( const float deltatime ) {
	for( uint16_t i{0}; i <= SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j <= SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			// Calculate maximum angle
			const float m1{ std::max( c.getTotalHeightDiff( grid[i-1][j] ), c.getTotalHeightDiff( grid[i+1][j] ) ) };
			const float m2{ std::max( c.getTotalHeightDiff( grid[i][j-1] ), c.getTotalHeightDiff( grid[i][j+1] ) ) };
			const float angle{ std::atan( omath::radians( std::max( m1, m2 ) / SETTINGS.distanceX_LX ) ) };
			// Transport capacity from tilt angle and flow velocity
			c.sedimentTransportCapacity_C = SETTINGS.sedimentCapacityConstant_KC *
					std::max( std::sin( angle ), SETTINGS.minFlowAngle ) * omath::magnitude( c.velocity_v );
			// Compare C with suspended sediment
			if( c.sedimentTransportCapacity_C >= c.tempSuspendedSediment_st ) {
				// Erosion
				const float t{ c.sedimentTransportCapacity_C - c.tempSuspendedSediment_st };
				c.terrainHeight_b -= SETTINGS.dissolvingConstant_KS * t;
				c.suspendedSedimentThisTurn_s1 += SETTINGS.dissolvingConstant_KS * t;
			} else {
				// Deposition
				const float t{ c.tempSuspendedSediment_st - c.sedimentTransportCapacity_C };
				c.terrainHeight_b += SETTINGS.depositionConstant_KD * t;
				c.suspendedSedimentThisTurn_s1 -= SETTINGS.depositionConstant_KD * t;
			}
		}
}

// Transport sediment with velocity field
void transportSediment( const float deltatime ) {
	for( uint16_t i{0}; i <= SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j <= SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			// @todo: check for boundaries, if not on grid interpolate the four neighbours
			c.sudpendedSediment_s = c.suspendedSedimentThisTurn_s1(
					i - c.velocity_v.x * deltatime, j - c.velocity_v.y * deltatime
			);
		}
}

void evaporateWater( const float deltatime ) {
	for( uint16_t i{0}; i <= SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j <= SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			c.waterHeight_d2 *= 1.0f - SETTINGS.evaporationConstant_KE * deltatime;
		}
}

void update() {
	// @todo time step determines stability !
	float deltatime{ 1.0f / 60.0f };
	// d1 from dt
	incrementWater( deltatime );
	// d2, f, v from d1, bt, ft
	calculateFlux( deltatime );
	calculateVelocityField( deltatime );
	updateWaterheight( deltatime );
	// bt, s1 from v, bt, st
	calculateErosionDeposition( deltatime );
	// st from s1, v
	transportSediment( deltatime );
	// dt from d2
	evaporateWater( deltatime );
}

private:
	cell_t **grid{nullptr};

	// @todo velocity field as own grid ?

};
