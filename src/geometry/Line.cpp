
#include <geometry/Line.h>
#include <geometry/OBB.h>
#include <geometry/Plane.h>
#include <geometry/Ray.h>

namespace orf_n {

Line::Line() {}

Line::Line( const omath::dvec3 &start, const omath::dvec3 &end ) :
		m_start{start}, m_end{end} {}

Line::~Line() {}

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


}
