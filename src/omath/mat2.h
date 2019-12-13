
#pragma once

#include <omath/vec3.h>

namespace omath {

template <typename T>
struct mat2_t {
	vec2_t<T> value[2];

	mat2_t() : value{ { 1, 0 }, { 0, 1 } } {}

	mat2_t( const mat2_t<T> &rhs ) = default;

	mat2_t( mat2_t<T> &&rhs ) = default;

	mat2_t &operator=( const mat2_t<T> &rhs ) = default;

	mat2_t &operator=( mat2_t<T> &&rhs ) = default;

	virtual ~mat2_t() {}

	vec2_t<T> &operator[]( const int i ) {
		return value[i];
	}

	const vec2_t<T> &operator[]( const int i ) const {
		return value[i];
	}

};

}
