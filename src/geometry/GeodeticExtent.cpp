
#include <geometry/GeodeticExtent.h>
#include <omath/common.h>

namespace orf_n {

GeodeticExtent::GeodeticExtent( const double &w, const double &s, const double &e, const double &n ) :
		m_west{ w }, m_south{ s }, m_east{ e }, m_north{ n } {}

GeodeticExtent::GeodeticExtent( const Geodetic &bl, const Geodetic &ur ) :
		m_west{ bl.getLongitude() }, m_south{ bl.getLatitude() },
		m_east{ ur.getLongitude() }, m_north{ ur.getLatitude() } {}

double GeodeticExtent::getWest() const {
	return m_west;
}

double GeodeticExtent::getSouth() const {
	return m_south;
}

double GeodeticExtent::getEast() const {
	return m_east;
}

double GeodeticExtent::getNorth() const {
	return m_north;
}

bool GeodeticExtent::operator==( const GeodeticExtent &right ) {
	return omath::compare_float( m_west, right.m_west ) &&
		   omath::compare_float( m_south, right.m_south ) &&
		   omath::compare_float( m_east, right.m_east ) &&
		   omath::compare_float( m_north, right.m_north );
}

bool GeodeticExtent::operator!=( const GeodeticExtent &right ) {
	return !( *this == right );
}

GeodeticExtent::~GeodeticExtent() {}

}
