
#pragma once

#include <geometry/geodetic.h>

namespace orf_n {

class GeodeticExtent {
public:
	/**
	 * Corners in degrees latitude and longitude
	 */
	GeodeticExtent( const double &w, const double &s, const double &e, const double &n );

	GeodeticExtent( const geodetic &bottomLeft, const geodetic &upperRight );

	virtual ~GeodeticExtent();

	double getWest() const;

	double getSouth() const;

	double getEast() const;

	double getNorth() const;

	bool operator==( const GeodeticExtent &other );

	bool operator!=( const GeodeticExtent &other );

private:
	double m_west;

	double m_south;

	double m_east;

	double m_north;


};

}
