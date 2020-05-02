
#include <geometry/Geodetic.h>
#include <omath/common.h>
#include <iostream>

namespace orf_n {

Geodetic::Geodetic() {}

Geodetic::Geodetic( const double lat, const double lon, const double h ) :
		m_longitude{ lon }, m_latitude{ lat }, m_height{ h } {}

// virtual
Geodetic::~Geodetic() {}

double Geodetic::getLongitude() const {
	return m_longitude;
}

double Geodetic::getLatitude() const {
	return m_latitude;
}

double Geodetic::getHeight() const {
	return m_height;
}

bool Geodetic::operator==( const Geodetic &other ) const {
	return omath::compare_float( m_longitude, other.m_longitude ) &&
		   omath::compare_float( m_latitude, other.m_latitude ) &&
		   omath::compare_float( m_height, other.m_height );
}

bool Geodetic::operator!=( const Geodetic &other ) const {
	return !( *this == other );
}

}

std::ostream &operator<<( std::ostream &o, const orf_n::Geodetic &geo ) {
	o << std::fixed << '(' << geo.getLatitude() << "rad lat/" << geo.getLongitude() << "rad lon" << ')';
	return o;
}
