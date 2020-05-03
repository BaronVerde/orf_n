
/**
 * A class for a triaxial allipsoid.
 * Comes with conversion routines for cartesian (e.g. WGS84) to geidetic (lat,lon,height)
 * coordinates.
 */

#pragma once

#include <geometry/geodetic.h>
#include <omath/vec3.h>
#include <vector>

namespace orf_n {

class ellipsoid {
public:
	static const omath::dvec3 WGS84_ELLIPSOID;
	static const omath::dvec3 WGS84_ELLIPSOID_SMALL;
	static const omath::dvec3 WGS84_ELLIPSOID_SCALED;
	static const omath::dvec3 UNIT_SPHERE;
	static const omath::dvec3 ONE_TO_FIVE;
	static const omath::dvec3 ONE_TO_TWO;
	static const omath::dvec3 ONE_TO_THREE;

	/**
	 * create a new ellipsoid with given radii as vector or doubles.
	 */
	ellipsoid( const omath::dvec3 &radii );

	ellipsoid( const double x, const double y, const double z );

	/**
	 * Surface normals from cartesian position. Just normalizes position. Normals are single prec.
	 */
	static omath::vec3 centric_surface_normal( const omath::dvec3 &cartesian_position );

	/**
	 * Geodetic surface normal from cartesian position.
	 */
	virtual omath::vec3 geodetic_surface_normal( const omath::dvec3 &cartesian_position ) const;

	/**
	 * Surface normal from geodetic position.
	 */
	static omath::vec3 geodetic_surface_normal( const geodetic &geodetic );

	/**
	 * Conversion from geodetic to cartesian position.
	 */
	virtual omath::dvec3 to_cartesian( const geodetic &geodetic ) const;

	/**
	 * Determine surface point of a cartesian coordinate along its geodetic normal.
	 * Iterative, should converge quickly (1-4 iterations).
	 * Could take several iterations for mor oblate ellipsoids, especially for more
	 * oblate ellipsoids.
	 * Assumes the ellipsoid is centered at the origin.
	 */
	virtual omath::dvec3 scale_to_geodetic_surface( const omath::dvec3 &cartesian_position ) const;

	/**
	 * Determine surface point of a cartesian coordinate along its geocentric normal.
	 * Assumes the ellipsoid is centered at the origin.
	 */
	virtual omath::dvec3 scale_to_geocentric_surface( const omath::dvec3 &cartesian_position ) const;

	/**
	 * Cartesian to geodetic conversion of arbitrary point.
	 */
	virtual geodetic to_geodetic( const omath::dvec3 &cartesian_position ) const;

	/**
	 * Common getters
	 */
	virtual const omath::dvec3 &get_radii() const;

	virtual void set_position( const omath::dvec3 &position );

	virtual const omath::dvec3 &get_position() const;

	virtual const omath::dvec3 &get_radii_squared() const;

	virtual const omath::dvec3 &getOneOverRadiiSquared() const;

	virtual double get_minimum_radius() const;

	virtual double get_maximum_radius() const;

	/**
	 * @todo Intesections of a vector with the ellipsoid hull. Either 0, 1(tangent) or 2.
	 */
	virtual std::vector<double> intersections( const omath::dvec3 origin, const omath::dvec3 direction ) const;

	/**
	 * Granularity ?
	 * @todo: Wip ! test the implementation.
	 */
	virtual std::vector<omath::dvec3> compute_curve(
			const omath::dvec3 start,
			const omath::dvec3 stop,
			const double granularity ) const;

	/**
	 * Converts texture coordinates
	 * Takes a geodetic surface normal normalized to [-1, 1] and computes
	 * s (horizontal) and t (vertical) coordinate as [0, 1]
	 */
	static omath::vec2 compute_texture_coordinate( const omath::vec3 &normal );

	virtual ~ellipsoid();

protected:
	omath::dvec3 m_position;

	omath::dvec3 m_radii;

	omath::dvec3 m_radii_squared;

	omath::dvec3 m_radii_to_the_fourth;

	omath::dvec3 m_one_over_radii_squared;

};

}
