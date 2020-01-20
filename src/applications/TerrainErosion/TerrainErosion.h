
#pragma once

#include <string>
#include <array>
#include "omath/vec2.h"

// @todo: consider uisng handles instead of pointers

static const float precipitationRate{1.0f};
static const float evaporationRate{0.5f};
static const float fluidDensity{1.0f};
//static const float gravitationalConstant{6.6743f}; // m³/(kg*s²)
static const float gravitationalAcceleration{9.81f}; // m/s²
static const float flowConstant{ fluidDensity * gravitationalAcceleration };
static const unsigned int numLayers{4};
static const unsigned int extent{256};
// distance between to columns on the map
static const float cellsize{10.0f};
static const float distance{ omath::distance( omath::vec2{ 0.0f, 0.0f }, omath::vec2{ 1.0f, 0.0f } ) * cellsize };
static const float crossSectionOfFlow{ distance * distance };

typedef enum { left = 0, top, right, bottom } positionDelta;

struct stratumMaterial {
	std::string name;
	// erodability
	float hardness;
	// e.g. 0.0001 for bedrock, 0.1 for sand (silt and clay could be higher again)
	float sedimentCapacityConstant;
};

struct stratum {
	stratumMaterial *material;
	float height;
};

struct column {
	std::array<stratum, numLayers> strata;
	// Offset of height above reference frame, could be replaced with an own stratum
	float offset{0.0f};
	float waterHeight;
	float outflow;

	// Returns sum of water height and the different layers
	float getColumnHeight() const {
		float erg{offset + waterHeight};
		for( size_t i{0}; i < strata.size(); ++i )
			erg += strata[i].height;
		return erg;
	}
};

static column **heightField{ nullptr };

// Returns static pressure difference between adjacent columns
static float getPressureDifference( const omath::uvec2 &position, const positionDelta delta ) {
	float diff{flowConstant};
	const column col{ heightField[position.x][position.y] };
	switch( delta ) {
		case left:
			diff *= position.x == 0 ? 0 : col.getColumnHeight() - heightField[position.x-1][position.y].getColumnHeight();
			break;
		case top:
			diff *= position.y == 0 ? 0 : col.getColumnHeight() - heightField[position.x][position.y-1].getColumnHeight();
			break;
		case right:
			diff *= position.x == extent ? 0 : col.getColumnHeight() - heightField[position.x+1][position.y].getColumnHeight();
			break;
		case bottom:
			diff *= position.y == extent ? 0 : col.getColumnHeight() - heightField[position.x][position.y+1].getColumnHeight();
			break;
		default:
			diff = 0.0f;
			break;
	}
	return diff;
}

// Calculates the pressure difference between two adjacent columns on the map. 0 at the edge
static float getAcceleration( const omath::uvec2 &position, const positionDelta delta ) {
	return getPressureDifference( position, delta ) / ( fluidDensity * distance );
}

// Change in flow in deltatime between two columns
static float updateFlowInPipe( const omath::uvec2 &position, const positionDelta delta, const float deltatime ) {
	heightField[position.x][position.y].outflow += deltatime * crossSectionOfFlow * getAcceleration( position, delta );
	return heightField[position.x][position.y].outflow;
}

// Calculates and updates waterheight in culomn from flow between two adjacent columns
static float updateWaterheight( omath::uvec2 &position, const float deltatime ) {
	const float oldOutflow{ heightField[position.x][position.y].outflow };
	float height{0.0f};
	height += heightField[position.x-1][position.y].outflow - oldOutflow;
	height += heightField[position.x][position.y-1].outflow - oldOutflow;
	height += heightField[position.x+1][position.y].outflow - oldOutflow;
	height += heightField[position.x][position.y+1].outflow - oldOutflow;
	height = heightField[position.x][position.y].waterHeight + deltatime / crossSectionOfFlow;
	// Negative height does not exist
	heightField[position.x][position.y].waterHeight = height <= 0.0f ? 0.0f : height;
	return heightField[position.x][position.y].waterHeight;
}


// Updates the amount of water leaving column
static float updateOutflow( omath::uvec2 &position, const float deltatime ) {
	float flow{0.0f};
	for( size_t i{0}; i < 3; ++i )
		flow += updateFlowInPipe( position, static_cast<positionDelta>(i), deltatime );
	// Flow can't be greater than height of watercolumn * cross section (here: distance*distance)
	if( flow > heightField[position.x][position.y].waterHeight * crossSectionOfFlow )
		flow = heightField[position.x][position.y].waterHeight * crossSectionOfFlow;
	heightField[position.x][position.y].outflow = flow;
	return flow;
}

// Calculate amount of water passing through a column in a direction (form opposite to delta)
// Assumes that flow in pipes has been computed before
static float calculateVelocityOfFlow( const omath::uvec2 &position, const positionDelta delta, const float deltatime ) {
	// calculate flow in x direction
	// 0.5 * ( flow( left to pos ) - flow( pos to left ) + flow( pos to right ) - flow( right to pos ) )
	// calculate velocity x vector
	// flow in x / ( distance * average of water height in column of pos during last two steps
	// same for y component
	return 0.0f;
}

static float calculateSedimentTransportCapacity( const omath::uvec2 &position, const float deltatime ) {
	// = |velocityOfFlow| * sedimentCapacityConstant of layer * sin(tilt angle of terrain)
	return 0.0f;
}
