
#include "view_frustum.h"
#include "omath/vec3.h"
#include <iostream>

namespace orf_n {

view_frustum::view_frustum() {}

view_frustum::~view_frustum() {}

void view_frustum::set_fov( const double& angle, const double& ratio, const double& nearD, const double& farD ) {
	m_ratio = ratio;
	m_near_d = nearD;
	m_far_d = farD;
	m_angle = omath::radians( angle );
	// compute width and height of the near plane for point intersection test
	m_tangens_angle = std::tan( m_angle * 0.5f );
	m_height = m_near_d * m_tangens_angle;
	m_width = m_height * m_ratio;
	// compute sphere factors for sphere intersection test
	m_sphere_factor_y = 1.0f / std::cos( m_angle );
	double anglex{ std::atan( m_tangens_angle * ratio ) };
	m_sphere_factor_x = 1.0f / std::cos( anglex );
}

void view_frustum::set_camera_vectors( const omath::dvec3& pos, const omath::dvec3& front, const omath::dvec3& up ) {
	m_camera_position = pos;
	m_z = omath::normalize( front - pos );
	m_x = omath::normalize( omath::cross( m_z, up ) );
	m_y = omath::cross( m_x, m_z );
}

intersect_t view_frustum::is_point_in_frustum( const omath::dvec3& point ) const {
	// compute vector from camera position to p
	omath::dvec3 v{ point - m_camera_position };
	// compute and test the z coordinate
	double pcz{ omath::dot( v, m_z ) };
	if( pcz > m_far_d || pcz < m_near_d )
		return OUTSIDE;
	// compute and test the m_y coordinate
	double pcy{ omath::dot( v, m_y ) };
	double aux{ pcz * m_tangens_angle };
	if( pcy > aux || pcy < -aux )
		return OUTSIDE;
	// compute and test the m_x coordinate
	double pcx{ omath::dot( v, m_x ) };
	aux *= m_ratio;
	if( pcx > aux || pcx < -aux )
		return OUTSIDE;
	return INSIDE;
}

intersect_t view_frustum::is_sphere_in_frustum( const omath::dvec3& center, const double& radius ) const {
	intersect_t result{ INSIDE };
	omath::dvec3 v{ center - m_camera_position };

	double az{ omath::dot( v, m_z ) };
	// Early out against near and far plane
	if( az > m_far_d + radius || az < m_near_d - radius )
		return OUTSIDE;
	if( az > m_far_d - radius || az < m_near_d + radius )
		result = INTERSECTS;

	double ay{ omath::dot( v, m_y ) };
	double d{ m_sphere_factor_y * radius };
	az *= m_tangens_angle;
	if( ay > az + d || ay < -az - d )
		return OUTSIDE;
	if( ay > az - d || ay < -az + d )
		result = INTERSECTS;

	double ax{ omath::dot( v, m_x ) };
	az *= m_ratio;
	d = m_sphere_factor_x * radius;
	if( ax > az + d || ax < -az - d )
		return OUTSIDE;
	if( ax > az-d || ax < -az+d )
		result = INTERSECTS;

	return result;
}

intersect_t view_frustum::is_box_in_frustum( const aabb& box ) const {
	// @todo I am so lazy ...
	return is_sphere_in_frustum( box.get_center(), box.get_diagonal_size() * 0.5 );
}

void view_frustum::print() const {
	std::cout << "View frusum:\ncamera position: " << m_camera_position << '\n' <<
			"view frustum x " << m_x << '\n' << "view frustum y " << m_y << '\n' <<
			"view frustum z " << m_z << '\n' << "Near " << m_near_d << " far " << m_far_d <<
			" width " << m_width << " height " << m_height << " ratio " << m_ratio <<
			std::endl;
}

/*
 * box/frustum intersection (fb frustum; ob box)
 * if (fb_xmin > ob_xmax || fb_xmax < ob_xmin || fb_ymin > ob_ymax || fb_ymax < ob_ymin ||
 * 		fb_zmin > ob_zmax || fb_zmax < ob_zmin)
	return (OUTSIDE);

else if (fb_xmin < ob_xmin& & fb_xmax > ob_xmax& &
		 fb_ymin < ob_ymin& & fb_ymax > ob_ymax& &
		 fb_zmin < ob_zmin& & fb_zmax > ob_zmax)
	return (INSIDE);
else
	return(INTERSECT);
 */

}
