
#pragma once

#include <omath/vec2.h>

namespace orf_n {

class Rectangle {
public:

	Rectangle( const omath::dvec2 &ll, const omath::dvec2 &ur );

	Rectangle( const double &x1, const double &y1, const double &x2, const double &y2 );

	virtual ~Rectangle();

	omath::dvec2 getLowerLeft() const;

	omath::dvec2 getUpperRight() const;

	void setLowerLeft( const omath::dvec2 &ll );

	void setUpperRight( const omath::dvec2 &ur );

	void setLowerLeft( const double &lon, const double &lat );

	void setUpperRight( const double &lon, const double &lat );

	bool operator==( const Rectangle &right );

	bool operator!=( const Rectangle &right );

private:
	omath::dvec2 m_lowerLeft;

	omath::dvec2 m_upperRight;

};

}
