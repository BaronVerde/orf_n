
#pragma once

#include "vec3.h"

class medium;

namespace omath {

template<typename T>
struct ray_t {

	vec3_t<T> m_origin;

	vec3_t<T> m_direction;

	// @todo can be changed if the ray is const (why ?)
	mutable T m_tmax{std::numeric_limits<T>::infinity()};

	T m_time{0};

	// Medium of ray's origin
	const medium *m_medium{nullptr};

	ray_t( const vec3_t<T> &o, const vec3_t<T> &d,
			T tmax = std::numeric_limits<T>::infinity(), T time = 0, const medium *med = nullptr ) :
			m_origin{o}, m_direction{d}, m_tmax{tmax}, m_time{time}, m_medium{med} {}

	ray_t( const ray_t<T> &rhs ) = default;

	ray_t( ray_t<T> &&rhs ) = default;

	ray_t &operator=( const ray_t<T> &rhs ) = default;

	ray_t &operator=( ray_t<T> &&rhs ) = default;

	// Function operator to get a point on a ray at value x
	vec3_t<T> operator()( const T x ) const {
		return m_origin + m_direction * x;
	}

};

//typedef ray_t<float> ray;

// Two extra rays are kept for antialiasing, representing camera rays with offsets in x and y direction
template<typename T>
struct ray_differential_t : public ray_t<T> {
	bool m_has_differentials{false};
	vec3_t<T> m_xorigin;
	vec3_t<T> m_yorigin;
	vec3_t<T> m_xdirection;
	vec3_t<T> m_ydirection;

	ray_differential_t() {
		m_has_differentials = false;
	}

	ray_differential_t( const vec3_t<T> &origin, const vec3_t<T> &direction,
			T tmax = std::numeric_limits<T>::infinity(), T time = 0, const medium *med = nullptr ) :
				ray_t<T>{ origin, direction, tmax, time, med } {
		// Neighbouring rays aren't known yet on construction
		// Camera class implements compute_differentials
		m_has_differentials = false;
	}

	ray_differential_t( const ray_differential_t<T> &rhs ) = default;

	ray_differential_t( ray_differential_t<T> &&rhs ) = default;

	ray_differential_t &operator=( const ray_differential_t<T> &rhs ) = default;

	ray_differential_t &operator=( ray_differential_t<T> &&rhs ) = default;

	void scale_differentials( const T s ) {
		m_xorigin = ray_t<T>::m_origin + ( m_xorigin - ray_t<T>::m_origin ) * s;
		m_yorigin = ray_t<T>::m_origin + ( m_yorigin - ray_t<T>::m_origin) * s;
		m_xdirection = ray_t<T>::m_direction + (m_xdirection - ray_t<T>::m_direction) * s;
		m_ydirection = ray_t<T>::m_direction + (m_ydirection - ray_t<T>::m_direction) * s;
	}

};

}
