
#include <geometry/Ray.h>

namespace orf_n {

Ray::Ray() : m_origin{ 0.0f, 0.0f, 0.0f }, m_direction{ 0.0f, 0.0f, 1.0f } {}

Ray::Ray( const omath::vec3 &origin, const omath::vec3 &direction ) :
	m_origin{origin}, m_direction{ omath::normalize( direction ) } {}

void Ray::normalizeDirection() {
	omath::normalize( m_direction );
}

bool Ray::raycast( const OBB &obb, raycastResult_t *outResult ) {
	//outResult->reset();
	const omath::vec3 p{ omath::vec3{obb.getPosition()} - m_origin };
	// Get orinetation axes. @todo: is this correct or use transposed vectors ?
	const omath::vec3 x{ obb.getOrientation()[0] };
	const omath::vec3 y{ obb.getOrientation()[1] };
	const omath::vec3 z{ obb.getOrientation()[2] };
	omath::vec3 f{ omath::dot( x, m_direction ), omath::dot( y, m_direction ), omath::dot( z, m_direction )	};
	omath::vec3 e{ omath::dot( x, p ), omath::dot( y, p ), omath::dot( z, p ) };
	if( omath::compareFloat( f.x, 0.0f ) ) {
		if( -e.x - obb.getHalfSize().x > 0 || -e.x + obb.getHalfSize().x < 0 )
			return false;
		// To avoid division by 0
		f.x = 0.00001f;
	} else if( omath::compareFloat( f.y, 0.0f ) ) {
		if( -e.y - obb.getHalfSize().y > 0 || -e.y + obb.getHalfSize().y < 0 )
			return false;
		f.y = 0.00001f;
	} else if( omath::compareFloat( f.z, 0.0f ) ) {
		if( -e.z - obb.getHalfSize().z > 0 || -e.z + obb.getHalfSize().z < 0 )
			return false;
		f.z = 0.00001f; // Avoid div by 0!
	}
	float t[]{ ( e.x + obb.getHalfSize().x ) / f.x,
			   ( e.x - obb.getHalfSize().x ) / f.x,
			   ( e.y + obb.getHalfSize().y ) / f.y,
			   ( e.y - obb.getHalfSize().y ) / f.y,
			   ( e.z + obb.getHalfSize().z ) / f.z,
			   ( e.z - obb.getHalfSize().z ) / f.z };
	float tmin{ std::max( std::max( std::min( t[0], t[1] ), std::min( t[2], t[3] ) ), std::min( t[4], t[5] ) ) };
	float tmax{ std::min( std::min( std::min( t[0], t[1] ), std::min( t[2], t[3] ) ), std::max( t[4], t[5] ) ) };
	// if tmax < 0, ray is intersecting box but entire box is behing it's origin
	if( tmax < 0 )
		return false;
		// if tmin > tmax, ray doesn't intersect box
	if( tmin > tmax )
		return false;
	// If tmin is < 0, tmax is closer
	float t_result{ tmin };
	if( tmin < 0.0f )
		t_result = tmax;
	if( outResult != 0 ) {
		outResult->hit = true;
		outResult->t = t_result;
		outResult->point = m_origin + m_direction * t_result;
		omath::vec3 normals[]{
				x,			// +x
				x * -1.0f,	// -x
				y,			// +y
				y * -1.0f,	// -y
				z,			// +z
				z * -1.0f	// -z
		};
		for( int i{0}; i < 6; ++i )
			if( omath::compareFloat( t_result, t[i] ) )
				outResult->normal = omath::normalize( normals[i] );
	}
	return true;
}

}
