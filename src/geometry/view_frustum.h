
/* The camera's view frustum, radar approach in world space
 * Source: http://www.lighthouse3d.com/tutorials/view-frustum-culling/ */

#pragma once

#include "aabb.h"
#include "omath/vec3.h"

namespace orf_n {

// @todo: UNDEFINED, OUT_OF_RANGE and SELECTED belong elsewhere
typedef enum {
	OUTSIDE, INTERSECTS, INSIDE, UNDEFINED, OUT_OF_RANGE, SELECTED
} intersect_t;

class view_frustum {
public:
	view_frustum();

	/**
	 * Must be called every time the lookAt matrix changes, e.g. on
	 * zoom-facter or near/far clip plane change. Angle in degrees.
	 */
	void set_fov( const double& angle, const double& ratio, const double& nearD, const double& farD);

	/**
	 * Must be called every time camera position or orientation changes,
	 * i. e. every frame.
	 * Takes unnormalized vectors just like the lookAt() matrix.
	 */
	void set_camera_vectors( const omath::dvec3& pos, const omath::dvec3& front, const omath::dvec3& up );

	virtual ~view_frustum();

	intersect_t is_point_in_frustum( const omath::dvec3& point ) const;

	intersect_t is_sphere_in_frustum( const omath::dvec3& center, const double& radius ) const;

	intersect_t is_box_in_frustum( const aabb& box ) const;

private:
	// store camera position and reference vectors for rapid use in intersection tests
	omath::dvec3 m_camera_position;
	omath::dvec3 m_x, m_y, m_z;

	double m_near_d;
	double m_far_d;
	double m_width;
	double m_height;
	double m_ratio;
	double m_tangens_angle;
	double m_angle;

	// precomputed angle for sphere intersection test.
	double m_sphere_factor_y;
	double m_sphere_factor_x;

};

}
