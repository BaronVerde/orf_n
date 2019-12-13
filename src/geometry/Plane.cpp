
#include <geometry/Plane.h>

namespace orf_n {

Plane::Plane() {}

Plane::Plane( const omath::dvec3 &origin, const omath::dvec3 &normal ) {
	setCoefficients( normal.x, normal.y, normal.z, -omath::dot( normal, origin ) );
}

Plane::Plane( const omath::dvec3 &normal, const double distance ) :
		m_normal{normal}, m_distance{distance} {}

Plane::Plane( const omath::dvec3 &v1, const omath::dvec3 &v2, const omath::dvec3 &v3 ) {
	set3Points( v1, v2, v3 );
}

Plane::Plane( const double &a, const double &b, const double &c, const double &d ) {
	setCoefficients( a, b, c, d );
}

Plane::~Plane() {}

void Plane::setNormalAndOrigin( const omath::dvec3 &normal, const omath::dvec3 &origin ) {
	m_normal = normal;
	m_distance = -omath::dot( normal, origin );
}

void Plane::set3Points( const omath::dvec3 &v1, const omath::dvec3 &v2, const omath::dvec3 &v3 ) {
	m_normal = omath::normalize( omath::cross( v1 - v2, v3 - v2 ) );
	m_distance = -omath::dot( m_normal, v2 );
}

void Plane::setCoefficients( const double &a, const double &b, const double &c, const double &d ) {
	m_normal.x = a;
	m_normal.y = b;
	m_normal.z = c;
	m_distance = d;
}

bool Plane::isFrontFacingTo( const omath::dvec3 &direction ) const {
	return omath::dot( m_normal, direction ) <= 0;
}

double Plane::signedDistanceTo( const omath::dvec3 &point ) const {
	return omath::dot( point, m_normal ) + m_distance;
}

}
