
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
