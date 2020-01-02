
#pragma once

#include <string>
#include <array>
#include "omath/vec2.h"

// @todo: consider uisng handles instead of pointers

static const float precipitationRate{1.0f};
static const float evaporationRate{0.5f};
static const float fluidDensity{1.0f};
static const float gravitationalConstant{6.6743f}; // m³/(kg*s²)
static const float differenceConst{ fluidDensity * gravitationalConstant };
static const unsigned int numLayers{4};
static const unsigned int extent{4096};
// distance between to columns on the map
static const float cellsize{10.0f};

typedef enum { left = 0, top, right, bottom } positionDelta;

struct stratumMaterial {
	std::string name;
	float hardness;
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

	float getHeight() const {
		float erg{offset + waterHeight};
		for( size_t i{0}; i < strata.size(); ++i )
			erg += strata[i].height;
		return erg;
	}
};

static column **heightField{ nullptr };

static float getPressureDifference( const omath::uvec2 &position, const positionDelta delta ) {
	float diff{differenceConst};
	const column col{ heightField[position.x][position.y] };
	switch( delta ) {
		case left:
			diff *= position.x == 0 ? 0 : col.getHeight() - heightField[position.x-1][position.y].getHeight();
			break;
		case top:
			diff *= position.y == 0 ? 0 : col.getHeight() - heightField[position.x][position.y-1].getHeight();
			break;
		case right:
			diff *= position.x == extent ? 0 : col.getHeight() - heightField[position.x+1][position.y].getHeight();
			break;
		case bottom:
			diff *= position.y == extent ? 0 : col.getHeight() - heightField[position.x][position.y+1].getHeight();
			break;
		default:
			diff = 0.0f;
			break;
	}
	return diff;
}

// calculates the pressure difference between two columns on the map
// positive: end1 is lower than end0, negative: end0 is lower than end1
static float getAcceleration( const omath::uvec2 &end0, const omath::uvec2 &end1 ) {
	if( end0 == end1 )
		return 0.0f;
	// @todo no bounds checking
	return ( differenceConst * ( heightField[end0.x][end0.y].getHeight() - heightField[end1.x][end1.y].getHeight() ) ) /
		   ( omath::distance( end0, end1 ) * cellsize );
}
