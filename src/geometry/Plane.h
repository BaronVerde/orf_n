
#pragma once

#include <omath/vec3.h>

namespace orf_n {

struct Plane {
	/**
	 * ctor empty plane
	 */
	Plane();

	/**
	 * ctor from origin and normal
	 */
	Plane( const omath::dvec3 &origin, const omath::dvec3 &normal );

	Plane( const omath::dvec3 &normal, const double distance );

	/**
	 * ctor from 3 points
	 */
	Plane( const omath::dvec3 &v1, const omath::dvec3 &v2, const omath::dvec3 &v3 );

	/**
	 * ctor from coefficients a, b, c, d
	 */
	Plane( const double &a, const double &b, const double &c, const double &d );

	virtual ~Plane();

	void setCoefficients( const double &a, const double &b, const double &c, const double &d );

	void setNormalAndOrigin( const omath::dvec3 &normal, const omath::dvec3 &origin );

	void set3Points( const omath::dvec3 &v1, const omath::dvec3 &v2, const omath::dvec3 &v3 );

	bool isFrontFacingTo( const omath::dvec3 &direction ) const;

	double signedDistanceTo( const omath::dvec3 &point ) const;

	omath::dvec3 m_normal{ omath::dvec3( 0.0, 0.0, 0.0 ) };

	double m_distance{ 0.0 };

};

}
