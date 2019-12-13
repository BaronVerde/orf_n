
#include <geometry/Rectangle.h>

namespace orf_n {

Rectangle::Rectangle( const omath::dvec2 &ll, const omath::dvec2 &ur ) :
		m_lowerLeft{ ll }, m_upperRight{ ur } {}

Rectangle::Rectangle( const double &x1, const double &y1, const double &x2, const double &y2 ) {
	Rectangle( omath::dvec2( x1, y1 ), omath::dvec2( x2, y2 ) );
}

Rectangle::~Rectangle() {}

omath::dvec2 Rectangle::getLowerLeft() const {
	return m_lowerLeft;
}

omath::dvec2 Rectangle::getUpperRight() const {
	return m_upperRight;
}

void Rectangle::setLowerLeft( const omath::dvec2 &ll ) {
	m_lowerLeft = ll;
}

void Rectangle::setUpperRight( const omath::dvec2 &ur ) {
	m_upperRight = ur;
}

void Rectangle::setLowerLeft( const double &lon, const double &lat ) {
	m_lowerLeft = omath::dvec2( lon, lat );
}

void Rectangle::setUpperRight( const double &lon, const double &lat  ) {
	m_upperRight = omath::dvec2( lon, lat );
}

bool Rectangle::operator==( const Rectangle &right ) {
	return this->m_lowerLeft == right.m_lowerLeft &&
		   this->m_upperRight == right.m_upperRight;
}

bool Rectangle::operator!=( const Rectangle &right ) {
	return !( *this == right );
}

}
