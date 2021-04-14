
#pragma once

#include <omath/vec3.h>

namespace orf_n {

class OBB;
class Plane;

class Line {
public:
	Line();

	Line( const omath::dvec3 &start, const omath::dvec3 &end );

	virtual ~Line();

	bool testOBB( const OBB &obb ) const;

	/**
	 * Checks if line intersects Plane and if it does returns the intersection point
	 */
	bool clipToPlane( const Plane& plane, omath::dvec3 &outPoint ) const;

private:
	omath::dvec3 m_start;

	omath::dvec3 m_end;

};

}

/*
bool Line::testOBB( const OBB &obb ) const {
	/*@todo if( magnitude( m_end - m_start ) < 0.0000001f )
		return obb.containsPoint( m_start );*/
	Ray ray{ m_start, omath::normalize( m_end - m_start ) };
	raycastResult_t result;
	if( !ray.raycast( obb, &result ) )
		return false;
	return result.t >= 0.0 && result.t * result.t <= magnitude_sq( m_end - m_start );
}

bool Line::clipToPlane( const Plane &plane, omath::dvec3 &outPoint ) const {
	const omath::dvec3 ab{ m_end - m_start };
	const double nAB{ omath::dot( plane.m_normal, ab ) };
	// Early out if no intersection
	if( omath::compare_float( nAB, 0.0 ) )
		return false;
	// Find intersection time along the line
	const double nA{ omath::dot( plane.m_normal, m_start ) };
	const double t{ ( plane.m_distance - nA ) / nAB };
	// If valid, return intersection point
	if( t >= 0.0 && t <= 1.0 ) {
		outPoint = m_start + ab * t;
		return true;
	}
	return false;
}
*/
