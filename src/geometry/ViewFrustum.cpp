
#include <geometry/ViewFrustum.h>

namespace orf_n {

ViewFrustum::ViewFrustum() {}

ViewFrustum::~ViewFrustum() {}

void ViewFrustum::setFOV( const double &angle, const double &ratio, const double &nearD, const double &farD ) {
	m_ratio = ratio;
	m_nearD = nearD;
	m_farD = farD;
	m_angle = omath::radians( angle );
	// compute width and height of the near plane for point intersection test
	m_tangensAngle = std::tan( m_angle * 0.5f );
	m_height = m_nearD * m_tangensAngle;
	m_width = m_height * m_ratio;
	// compute sphere factors for sphere intersection test
	m_sphereFactorY = 1.0f / std::cos( m_angle );
	double anglex{ std::atan( m_tangensAngle * ratio ) };
	m_sphereFactorX = 1.0f / std::cos( anglex );
}

void ViewFrustum::setCameraVectors( const omath::dvec3 &pos, const omath::dvec3 &front, const omath::dvec3 &up ) {
	m_cameraPosition = pos;
	m_z = omath::normalize( front - pos );
	m_x = omath::normalize( omath::cross( m_z, up ) );
	m_y = omath::cross( m_x, m_z );
}

intersect_t ViewFrustum::isPointInFrustum( const omath::dvec3 &point ) const {
	// compute vector from camera position to p
	omath::dvec3 v{ point - m_cameraPosition };
	// compute and test the z coordinate
	double pcz{ omath::dot( v, m_z ) };
	if( pcz > m_farD || pcz < m_nearD )
		return OUTSIDE;
	// compute and test the m_y coordinate
	double pcy{ omath::dot( v, m_y ) };
	double aux{ pcz * m_tangensAngle };
	if( pcy > aux || pcy < -aux )
		return OUTSIDE;
	// compute and test the m_x coordinate
	double pcx{ omath::dot( v, m_x ) };
	aux *= m_ratio;
	if( pcx > aux || pcx < -aux )
		return OUTSIDE;
	return INSIDE;
}

intersect_t ViewFrustum::isSphereInFrustum( const omath::dvec3 &center, const double &radius ) const {
	intersect_t result{ INSIDE };
	omath::dvec3 v{ center - m_cameraPosition };

	double az{ omath::dot( v, m_z ) };
	// Early out against near and far plane
	if( az > m_farD + radius || az < m_nearD - radius )
		return OUTSIDE;
	if( az > m_farD - radius || az < m_nearD + radius )
		result = INTERSECTS;

	double ay{ omath::dot( v, m_y ) };
	double d{ m_sphereFactorY * radius };
	az *= m_tangensAngle;
	if( ay > az + d || ay < -az - d )
		return OUTSIDE;
	if( ay > az - d || ay < -az + d )
		result = INTERSECTS;

	double ax{ omath::dot( v, m_x ) };
	az *= m_ratio;
	d = m_sphereFactorX * radius;
	if( ax > az + d || ax < -az - d )
		return OUTSIDE;
	if( ax > az-d || ax < -az+d )
		result = INTERSECTS;

	return result;
}

intersect_t ViewFrustum::isBoxInFrustum( const AABB *const box ) const {
	// @todo I am so lazy ...
	return isSphereInFrustum( box->getCenter(), box->getDiagonalSize() * 0.5 );
}

}

/*
 * box/frustum intersection (fb frustum; ob box)
 * if (fb_xmin > ob_xmax || fb_xmax < ob_xmin || fb_ymin > ob_ymax || fb_ymax < ob_ymin ||
 * 		fb_zmin > ob_zmax || fb_zmax < ob_zmin)
	return (OUTSIDE);

else if (fb_xmin < ob_xmin && fb_xmax > ob_xmax &&
		 fb_ymin < ob_ymin && fb_ymax > ob_ymax &&
		 fb_zmin < ob_zmin && fb_zmax > ob_zmax)
	return (INSIDE);
else
	return(INTERSECT);
 */
