
#pragma once

#include "omath/vec3.h"

namespace orf_n {

class Sphere {
public:
	Sphere() {}

	Sphere( const omath::dvec3 &position, const double &radius ) :
		m_position{position}, m_radius{radius} {}

	virtual ~Sphere() {}

	void setPosition( const omath::dvec3 &position ) {
		m_position = position;
	}

	void setRadius( const double &radius ) {
		m_radius = radius;
	}

	const omath::dvec3 &getPosition() const {
		return m_position;
	}

	const double &getRadius() const {
		return m_radius;
	}

private:
	omath::dvec3 m_position{ 0.0, 0.0, 0.0 };

	double m_radius{ 0.5 };

};

}
