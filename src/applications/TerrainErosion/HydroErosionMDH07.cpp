
#include "HydroErosionMDH07.h"
#include <iomanip>

HydroErosionMDH07::HydroErosionMDH07() : orf_n::renderable( "HydroErosionMDH07" ) {}

HydroErosionMDH07::~HydroErosionMDH07() {}

void HydroErosionMDH07::render() {
	// @todo time step determines stability !
	const float deltatime{ 1.0f / 60.0f };
	const float globalLevel{ 0.1f };
	resetWaterTable( globalLevel );
	outputHeight();
	// d1 from dt
	incrementWater( deltatime );
	outputTempWater();
	// d2, f, v from d1, bt, ft
	calculateFlux( deltatime );
	outputFlux();
	calculateVelocityField( deltatime );
	outputVelocity();
	updateWaterheight( deltatime );
	// bt, s1 from v, bt, st
	calculateErosionDeposition();
	// st from s1, v
	transportSediment( deltatime );
	// dt from d2
	evaporateWater( deltatime );
}

// Reset watertable on boundaries to "global level"
void HydroErosionMDH07::resetWaterTable( const float globalLevel ) {
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j )
			if( i == 0 || j == 0 )
				grid[i][j].waterHeight_d2 = globalLevel;
}

// Increase due to rain or river flow
void HydroErosionMDH07::incrementWater( const float deltatime ) {
	// @todo: this can be result of some more complex process
	float waterThisTurn_r{ SETTINGS.aDropOfRain * SETTINGS.rainMultiplier };
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j ) {
			float inc{ grid[i][j].waterHeight_d2 + deltatime * waterThisTurn_r };
			grid[i][j].tempWaterHeight_d1 = inc;
		}
}

void HydroErosionMDH07::calculateFlux( const float deltatime ) {
	const float g{ SETTINGS.gravity };
	const float pl{ SETTINGS.pipeLength };
	const float flowTime{ deltatime * SETTINGS.pipeCrossSection };
	// Calculate each cell's outflow flux to the 4 adjacent cells by total height differences,
	// pipe length/cross section. Limit flow to water in the cell via scaling factor K
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j ) {
			// Readability
			cell_t *c{ &grid[i][j] };
			// Flux at the boundaries is set to 0.0f
			if( i == 0 )
				c->flux_f[left] = 0.0f;
			else {
				const float flow{ flowTime * ( ( g * c->getTotalHeightDiff( grid[i-1][j] ) ) / pl ) };
				c->flux_f[left] = std::max( 0.0f, c->flux_f[left] ) + flow;
			}
			if( i == SETTINGS.dimension - 1 )
				c->flux_f[right] = 0.0f;
			else {
				const float flow{ flowTime * ( ( g * c->getTotalHeightDiff( grid[i+1][j] ) ) / pl ) };
				c->flux_f[right] = std::max( 0.0f, c->flux_f[right] ) + flow;
			}
			if( j == 0 )
				c->flux_f[top] = 0.0f;
			else {
				const float flow{ flowTime * ( ( g * c->getTotalHeightDiff( grid[i][j-1] ) ) / pl ) };
				c->flux_f[top] = std::max( 0.0f, c->flux_f[top] ) + flow;
			}
			if( j == SETTINGS.dimension - 1 )
				c->flux_f[bottom] = 0.0f;
			else {
				const float flow{ flowTime * ( ( g * c->getTotalHeightDiff( grid[i][j+1] ) ) / pl ) };
				c->flux_f[bottom] = std::max( 0.0f, c->flux_f[bottom] ) + flow;
			}
			const float scalingFactor_K{
				std::min( 1.0f, ( c->tempWaterHeight_d1 * SETTINGS.pipeCrossSection ) /
						( deltatime * ( c->flux_f[left] + c->flux_f[right] + c->flux_f[top] + c->flux_f[bottom] ) ) )
			};
			c->flux_f[left] *= scalingFactor_K;
			c->flux_f[right] *= scalingFactor_K;
			c->flux_f[top] *= scalingFactor_K;
			c->flux_f[bottom] *= scalingFactor_K;
		}
}

void HydroErosionMDH07::updateWaterheight( const float deltatime ) {
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			const float volumeChange{ deltatime *
				( grid[i-1][j].flux_f[right] + grid[i+1][j].flux_f[left] +
				grid[i][j-1].flux_f[bottom] + grid[i][j+1].flux_f[top ] ) -
				( c.flux_f[left] + c.flux_f[right] + c.flux_f[top] + c.flux_f[bottom] ) };
			c.waterHeight_d2 = c.tempWaterHeight_d1 + ( volumeChange / SETTINGS.pipeCrossSection );
		};
}

void HydroErosionMDH07::calculateVelocityField( const float deltatime ) {
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j ) {
			cell_t *c{ &grid[i][j] };
			// Water flowing in x direction. @todo Flow is set to 0 at boundaries. There must be a better way
			const float averagePassingWater_WX{
				( grid[i == 0 ? 0 : i-1][j].flux_f[right] - c->flux_f[left] +
				  grid[i == SETTINGS.dimension ? SETTINGS.dimension : i][j].flux_f[left] - c->flux_f[right] ) / 2.0f
			};
			const float averagePassingWater_WY{
				( grid[i][j == 0 ? 0 : j-1].flux_f[bottom] - c->flux_f[top] +
				  grid[i][j == SETTINGS.dimension ? SETTINGS.dimension : j+1].flux_f[top] - c->flux_f[bottom] ) / 2.0f
			};
			// @todo Should be waterheigt of last two turns. Eventually extra pass.
			const float meanWaterHeight{ ( c->tempWaterHeight_d1 + c->waterHeight_d2 ) / 2.0f };
			c->velocity_v.x = averagePassingWater_WX / ( SETTINGS.distanceX_LX * meanWaterHeight );
			c->velocity_v.y = averagePassingWater_WY / ( SETTINGS.distanceY_LY * meanWaterHeight );
			// @todo: Stability condition: adjust time step to smaller values for smaller distances in grid
			if( deltatime * averagePassingWater_WX > SETTINGS.distanceX_LX ||
				deltatime * averagePassingWater_WY > SETTINGS.distanceY_LY )
				std::cout << "Time step too long, simulation unstable\n";
		}
}

void HydroErosionMDH07::calculateErosionDeposition() {
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j ) {
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
void HydroErosionMDH07::transportSediment( const float deltatime ) {
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			// @todo: check for boundaries, if not on grid interpolate the four neighbours
			const omath::vec2 v{c.velocity_v};
			c.sudpendedSediment_s =
					grid[i - (uint16_t)(v.x * deltatime)][j - (uint16_t)(v.y * deltatime)].suspendedSedimentThisTurn_s1;
		}
}

void HydroErosionMDH07::evaporateWater( const float deltatime ) {
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j ) {
			cell_t c{ grid[i][j] };
			c.waterHeight_d2 *= 1.0f - SETTINGS.evaporationConstant_KE * deltatime;
		}
}

void HydroErosionMDH07::setup() {
	grid = new cell_t *[SETTINGS.dimension];
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		grid[i] = new cell_t[SETTINGS.dimension];
	grid[1][1].terrainHeight_b = 0.1f; grid[1][2].terrainHeight_b = 0.1f; grid[1][3].terrainHeight_b = 0.1f; grid[1][4].terrainHeight_b = 0.1f; grid[1][5].terrainHeight_b = 0.1f; grid[1][6].terrainHeight_b = 0.1f;
	grid[2][1].terrainHeight_b = 0.1f; grid[2][2].terrainHeight_b = 0.3f; grid[2][3].terrainHeight_b = 0.3f; grid[2][4].terrainHeight_b = 0.3f; grid[2][5].terrainHeight_b = 0.3f; grid[2][6].terrainHeight_b = 0.1f;
	grid[3][1].terrainHeight_b = 0.1f, grid[3][2].terrainHeight_b = 0.3f; grid[3][3].terrainHeight_b = 0.5f; grid[3][4].terrainHeight_b = 0.5f; grid[3][5].terrainHeight_b = 0.2f; grid[3][6].terrainHeight_b = 0.1f;
	grid[4][1].terrainHeight_b = 0.1f; grid[4][2].terrainHeight_b = 0.3f; grid[4][3].terrainHeight_b = 0.5f; grid[4][4].terrainHeight_b = 0.5f; grid[4][5].terrainHeight_b = 0.2f; grid[4][6].terrainHeight_b = 0.1f;
	grid[5][1].terrainHeight_b = 0.1f; grid[5][2].terrainHeight_b = 0.3f; grid[5][3].terrainHeight_b = 0.2f; grid[5][4].terrainHeight_b = 0.2f; grid[5][5].terrainHeight_b = 0.2f; grid[5][6].terrainHeight_b = 0.1f;
	grid[6][1].terrainHeight_b = 0.1f; grid[6][2].terrainHeight_b = 0.1f; grid[6][3].terrainHeight_b = 0.1f; grid[6][4].terrainHeight_b = 0.1f; grid[6][5].terrainHeight_b = 0.1f; grid[6][6].terrainHeight_b = 0.1f;
}

void HydroErosionMDH07::outputHeight() {
	std::cout << "Terrain Height:\n" << std::fixed;
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i ) {
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j )
			std::cout << grid[i][j].terrainHeight_b << ' ';
		std::cout << std::endl;
	}
}

void HydroErosionMDH07::outputTempWater() {
	std::cout << "Temp Water:\n" << std::fixed;
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i ) {
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j )
			std::cout << grid[i][j].tempWaterHeight_d1 << ' ';
		std::cout << std::endl;
	}
}

void HydroErosionMDH07::outputFlux() {
	std::cout << "Flux:\n" << std::fixed;
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i ) {
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j )
			std::cout << grid[i][j].flux_f[left] << '/' << grid[i][j].flux_f[right] << '/' <<
			grid[i][j].flux_f[top] << '/' << grid[i][j].flux_f[bottom] << ' ';
		std::cout << std::endl;
	}
}

void HydroErosionMDH07::outputVelocity() {
	std::cout << "Velocity:\n" << std::fixed;
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i ) {
		for( uint16_t j{0}; j < SETTINGS.dimension; ++j )
			std::cout << grid[i][j].velocity_v.x << '/' << grid[i][j].velocity_v.y << ' ';
		std::cout << std::endl;
	}
}

void HydroErosionMDH07::cleanup() {
	for( uint16_t i{0}; i < SETTINGS.dimension; ++i )
		delete [] grid[i];
	delete [] grid;
}

/*
 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
 0.0f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.0f
 0.0f, 0.1f, 0.3f, 0.3f, 0.3f, 0.3f, 0.1f, 0.0f
 0.0f, 0.1f, 0.3f, 0.5f, 0.5f, 0.2f, 0.1f, 0.0f
 0.0f, 0.1f, 0.3f, 0.5f, 0.5f, 0.2f, 0.1f, 0.0f
 0.0f, 0.1f, 0.3f, 0.2f, 0.2f, 0.2f, 0.1f, 0.0f
 0.0f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.0f
 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
 */
