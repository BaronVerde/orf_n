
#pragma once

#include "vec3.h"

namespace omath {

// Just another aabb in 3d
template<typename T>
struct bounds3_t {

	vec3_t<T> m_min, m_max;

	// aabb with a single point
	bounds3_t(const vec3_t<T> &p) : m_min(p), m_max(p) {}

	bounds3_t() {
		const T minNum = std::numeric_limits<T>::lowest();
		const T maxNum = std::numeric_limits<T>::max();
	    m_min = vec3_t<T>(maxNum, maxNum, maxNum);
	    m_max = vec3_t<T>(minNum, minNum, minNum);
	}

	// Find component min's and max's if necessary
	bounds3_t( const vec3_t<T> &p1, const vec3_t<T> &p2 ) :
		m_min( std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z) ),
		m_max( std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z) ) {}

	const vec3_t<T> &operator[](int i) const {
		return 0 == i ? m_min : m_max;
	}

	vec3_t<T> &operator[](int i) {
		return 0 == i ? m_min : m_max;
	}

	// Coordinates of one of the 8 corners
	vec3_t<T> corner( int corner ) const {
		return vec3_t<T>(
				(*this)[(corner & 1)].x, (*this)[(corner & 2) ? 1 : 0].y, (*this)[(corner & 4) ? 1 : 0].z
		);
	}

	// Returns bounding box including box and point
	bounds3_t<T> unite( const vec3_t<T> &p ) const {
		return bounds3_t<T>{
				vec3_t<T>{ std::min(m_min.x, p.x), std::min(m_min.y, p.y), std::min(m_min.z, p.z) },
				vec3_t<T>{ std::max(m_max.x, p.x), std::max(m_max.y, p.y), std::max(m_max.z, p.z) }
		};
	}

	// Returns bounding box including box and other box
	bounds3_t<T> unite( const bounds3_t<T> &b2 ) const {
		return bounds3_t<T>{
			vec3_t<T>{ std::min(m_min.x, b2.m_min.x), std::min(m_min.y, b2.m_min.y), std::min(m_min.z, b2.m_min.z) },
			vec3_t<T>{ std::max(m_max.x, b2.m_max.x), std::max(m_max.y, b2.m_max.y), std::max(m_max.z, b2.m_max.z) }
		};
	}

	bounds3_t<T> intersect( const bounds3_t<T> &b2 ) const {
		return bounds3_t<T>{
			vec3_t<T>{ std::max(m_min.x, b2.m_min.x), std::max(m_min.y, b2.m_min.y), std::max(m_min.z, b2.m_min.z) },
			vec3_t<T>{ std::min(m_max.x, b2.m_max.x), std::min(m_max.y, b2.m_max.y), std::min(m_max.z, b2.m_max.z) }
		};
	}

	bool overlaps( const bounds3_t<T> &b2 ) const {
		bool x = (m_max.x >= b2.m_min.x) && (m_min.x <= b2.m_max.x);
	    bool y = (m_max.y >= b2.m_min.y) && (m_min.y <= b2.m_max.y);
	    bool z = (m_max.z >= b2.m_min.z) && (m_min.z <= b2.m_max.z);
	    return (x && y && z);
	}

	bool inside( const vec3_t<T> &p, const bounds3_t<T> &b ) const {
		return ( p.x >= b.m_min.x && p.x <= b.m_max.x &&
	             p.y >= b.m_min.y && p.y <= b.m_max.y &&
	             p.z >= b.m_min.z && p.z <= b.m_max.z );
	}

	bounds3_t<T> expand( T delta ) const {
		return bounds3_t<T>{
			m_min - vec3_t<T>{delta, delta, delta},
			m_max + vec3_t<T>{delta, delta, delta}
		};
	}

	vec3_t<T> diagonal() const {
		return m_max - m_min;
	}

	T surface_area() const {
		const vec3_t<T> d{ diagonal() };
	    return (T)2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	T volume() const {
		vec3_t<T> d{ diagonal() };
		return d.x * d.y * d.z;
	}

	// Returns index of longest axis
	int maximum_extent() const {
		vec3_t<T> d{ diagonal() };
		if( d.x > d.y && d.x > d.z )
	        return 0;
	    else if (d.y > d.z)
	        return 1;
	    else
	        return 2;
	}

	// Lerps between corners of the box by given amount in each dimension
	vec3_t<T> Lerp(const vec3_t<T> &t) const {
	    return vec3_t<T>{
	    	lerp(t.x, m_min.x, m_max.x), lerp(t.y, m_min.y, m_max.y), lerp(t.z, m_min.z, m_max.z)
	    };
	}

	// Position of point relative to box
	vec3_t<T> offset( const vec3_t<T> &p) const {
		vec3_t<T> o = p - m_min;
		if (m_max.x > m_min.x)
			o.x /= m_max.x - m_min.x;
	    if (m_max.y > m_min.y)
	    	o.y /= m_max.y - m_min.y;
	    if (m_max.z > m_min.z)
	    	o.z /= m_max.z - m_min.z;
	    return o;
	}

	void bounding_sphere( vec3_t<T> *center, T *radius ) const {
		*center = (m_min + m_max) / (T)2;
		*radius = inside( *center ) ? distance(*center, m_max) : 0;
	}

};

}
