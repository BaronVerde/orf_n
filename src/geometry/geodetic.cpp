
#include <geometry/geodetic.h>
#include <omath/common.h>
#include <iostream>

namespace orf_n {

geodetic::geodetic() {}

geodetic::geodetic( const double lat, const double lon, const double h ) :
		m_longitude{ lon }, m_latitude{ lat }, m_height{ h } {}

// virtual
geodetic::~geodetic() {}

double geodetic::get_longitude() const {
	return m_longitude;
}

double geodetic::get_latitude() const {
	return m_latitude;
}

double geodetic::get_height() const {
	return m_height;
}

bool geodetic::operator==( const geodetic &other ) const {
	return omath::compare_float( m_longitude, other.m_longitude ) &&
		   omath::compare_float( m_latitude, other.m_latitude ) &&
		   omath::compare_float( m_height, other.m_height );
}

bool geodetic::operator!=( const geodetic &other ) const {
	return !( *this == other );
}

}

std::ostream &operator<<( std::ostream &o, const orf_n::geodetic &geo ) {
	o << std::fixed << '(' << geo.get_latitude() << "rad lat/" << geo.get_longitude() << "rad lon" << ')';
	return o;
}
