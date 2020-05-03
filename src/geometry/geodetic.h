
#pragma once

#include <iostream>

namespace orf_n {

class geodetic {
public:
	geodetic();

	/**
	 * Longitude and latitude in radians, height in meters above reference ellipsoid
	 */
	geodetic( const double lat, const double lon, const double h = 0.0 );

	double get_longitude() const;

	double get_latitude() const;

	double get_height() const;

	/**
	 * Equality within double epsilon()
	 */
	bool operator==( const geodetic &other ) const;

	bool operator!=( const geodetic &other ) const;

	virtual ~geodetic();

private:
	double m_longitude{ 0.0 };

	double m_latitude{ 0.0 };

	double m_height{ 0.0 };

};

}

std::ostream &operator<<( std::ostream &o, const orf_n::geodetic &geo );
