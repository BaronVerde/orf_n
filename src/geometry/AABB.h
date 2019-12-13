
/**
 * An axis aligned bounding box. Rather memory intensive.
 * @todo Eventually store 2 corners only and return the array only on demand.
 */

#pragma once

#include <omath/vec3.h>
#include <ostream>

namespace orf_n {

struct AABB {

	// member vars
	omath::dvec3 m_min;

	omath::dvec3 m_max;

	AABB();

	AABB( const omath::dvec3 &min, const omath::dvec3 &max );

	virtual ~AABB();

	/**
	 * Returns the center of the bounding box
	 */
	const omath::dvec3 getCenter() const;

	const omath::dvec3 getSize() const;

	double getDiagonalSize() const;

	bool intersectOther( const AABB &other ) const;

	double minDistanceFromPointSq( const omath::dvec3 &point ) const;

	double maxDistanceFromPointSq( const omath::dvec3 &point ) const;

	bool intersectSphereSq( const omath::dvec3 &center, double radiusSq ) const;

	bool isInsideSphereSq( const omath::dvec3 & center, double radiusSq ) const;

	// @todo needs overwork to work with vectors
	bool intersectRay( const omath::dvec3 &rayOrigin, const omath::dvec3 &rayDirection, double &distance ) const;
	/**
	 * Returns AABB that encloses this one and the other one
	 */
	AABB encloseOther( const AABB &other ) const;

	bool operator==( const AABB &other ) const;

	double getBoundingSphereRadius() const;

	omath::dvec3 getVertexPositive( const omath::vec3 &normal ) const;

	omath::dvec3 getVertexNegative( const omath::vec3 &normal ) const;

	AABB expand( double percentage ) const;

};

}

std::ostream &operator<<( std::ostream &o, const orf_n::AABB &b );
