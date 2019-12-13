
/**
 * The camera's view frustum, radar approach in world space
 * Source: http://www.lighthouse3d.com/tutorials/view-frustum-culling/
 * @todo: this does not necessarily have to use double precisison !
 */

#pragma once

#include <geometry/AABB.h>
#include <omath/vec3.h>

namespace orf_n {

/**
 * @todo: UNDEFINED, OUT_OF_RANGE and SELECTED belong elsewhere
 */
typedef enum {
	OUTSIDE, INTERSECTS, INSIDE, UNDEFINED, OUT_OF_RANGE, SELECTED
} intersect_t;

class ViewFrustum {
public:
	ViewFrustum();

	/**
	 * Must be called every time the lookAt matrix changes, e.g. on
	 * zoom-facter or near/far clip plane change. Angle in degrees.
	 */
	void setFOV( const double &angle, const double &ratio, const double &nearD, const double &farD);

	/**
	 * Must be called every time camera position or orientation changes,
	 * i. e. every frame.
	 * Takes unnormalized vectors just like the lookAt() matrix.
	 */
	void setCameraVectors( const omath::dvec3 &pos, const omath::dvec3 &front, const omath::dvec3 &up );

	virtual ~ViewFrustum();

	intersect_t isPointInFrustum( const omath::dvec3 &point ) const;

	intersect_t isSphereInFrustum( const omath::dvec3 &center, const double &radius ) const;

	intersect_t isBoxInFrustum( const AABB *const box ) const;

private:
	/**
	 * Store camera position and reference vectors for rapid use in intersection tests
	 */
	omath::dvec3 m_cameraPosition;
	omath::dvec3 m_x, m_y, m_z;

	double m_nearD;
	double m_farD;
	double m_width;
	double m_height;
	double m_ratio;
	double m_tangensAngle;
	double m_angle;

	/**
	 * Precomputed angle for sphere intersection test.
	 */
	double m_sphereFactorY;
	double m_sphereFactorX;

};

}
