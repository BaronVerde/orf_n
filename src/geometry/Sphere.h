
#pragma once

#include <omath/vec3.h>

namespace orf_n {

class Sphere {
public:
	Sphere() {}

	Sphere( const omath::vec3 &position, const float &radius ) :
		m_position{position}, m_radius{radius} {}

	virtual ~Sphere() {}

	void setPosition( const omath::vec3 &position ) {
		m_position = position;
	}

	void setRadius( const float &radius ) {
		m_radius = radius;
	}

	const omath::vec3 &getPosition() const {
		return m_position;
	}

	const float &getRadius() const {
		return m_radius;
	}

private:
	omath::vec3 m_position{ 0.0f, 0.0f, 0.0f };

	float m_radius{ 0.5f };

};

}
