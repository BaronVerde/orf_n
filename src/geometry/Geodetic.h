
#pragma once

#include <iostream>

namespace orf_n {

class Geodetic {
public:
	Geodetic();

	/**
	 * Longitude and latitude in radians, height in meters above reference ellipsoid
	 */
	Geodetic( const double lat, const double lon, const double h = 0.0 );

	double getLongitude() const;

	double getLatitude() const;

	double getHeight() const;

	/**
	 * Equality within double epsilon()
	 */
	bool operator==( const Geodetic &other ) const;

	bool operator!=( const Geodetic &other ) const;

	virtual ~Geodetic();

private:
	double m_longitude{ 0.0 };

	double m_latitude{ 0.0 };

	double m_height{ 0.0 };

};

}

std::ostream &operator<<( std::ostream &o, const orf_n::Geodetic &geo );
