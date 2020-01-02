
#pragma once

#include <omath/mat2.h>
#include <omath/vec3.h>

namespace omath {

template<typename T>
struct mat3_t {
	vec3_t<T> value[3];

	mat3_t() :
		value{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } } {}

	mat3_t( const mat3_t<T> &rhs ) = default;

	mat3_t( mat3_t<T> &&rhs ) = default;

	mat3_t &operator=( const mat3_t<T> &rhs ) = default;

	mat3_t &operator=( mat3_t<T> &&rhs ) = default;

	explicit mat3_t( const T &s ) :
		value{ { s, 0, 0 }, { 0, s, 0 }, { 0, 0, s } } {}

	explicit mat3_t( const T &x0, const T &y0, const T &z0,
					 const T &x1, const T &y1, const T &z1,
					 const T &x2, const T &y2, const T &z2 ) :
		value{ { x0, x1, x2 }, { y0, y1, y2 }, { z0, z1, z2 } } {}

	mat3_t(	const vec3_t<T> &v0,
			const vec3_t<T> &v1,
			const vec3_t<T> &v2 ) :
		value{ v0, v1, v2 } {}

	// Access to column vectors @todo bounds checking
	vec3_t<T> &operator[]( int const& i ) {
		return value[i];
	}

	const vec3_t<T> &operator[]( int const& i) const {
		return value[i];
	}

	// -- Conversions --
	template <typename U>
	mat3_t(	U const& x0, U const& y0, U const& z0,
			U const& x1, U const& y1, U const& z1,
			U const& x2, U const& y2, U const& z2 ) :
		value{ { x0, x1, x2 }, { y0, y1, y2 }, { z0, z1, z2 } } {}

	template <typename U>
	mat3_t(	vec3_t<U> const& v0,
			vec3_t<U> const& v1,
			vec3_t<U> const& v2 ) :
		value{ v0, v1, v2 } {}

	// -- Matrix conversions --
	template<typename U>
	explicit mat3_t( mat3_t<U> const& m ) :
		value{ m[0], m[1], m[2] } {}

	template<typename U>
	explicit mat3_t( mat2_t<U> const& m ) :
		value{ { m[0], 0 }, { m[1], 0 }, { 0, 0, 1 } } {}

	// -- Unary arithmetic operators --
	template<typename U>
	mat3_t &operator=( mat3_t<U> const& m ) {
		value[0] = m[0];
		value[1] = m[1];
		value[2] = m[2];
		return *this;
	}

	template<typename U>
	mat3_t &operator+=( U const& s) {
		value[0] += s;
		value[1] += s;
		value[2] += s;
		return *this;
	}

	template<typename U>
	mat3_t &operator+=( mat3_t<U> const& m ) {
		value[0] += m[0];
		value[1] += m[1];
		value[2] += m[2];
		return *this;
	}

	template<typename U>
	mat3_t &operator-=( U const& s) {
		value[0] -= s;
		value[1] -= s;
		value[2] -= s;
		return *this;
	}

	template<typename U>
	mat3_t &operator-=( mat3_t<U> const& m ) {
		value[0] -= m[0];
		value[1] -= m[1];
		value[2] -= m[2];
		return *this;
	}

	template<typename U>
	mat3_t &operator*=( U const& s) {
		value[0] *= s;
		value[1] *= s;
		value[2] *= s;
		return *this;
	}

	template<typename U>
	mat3_t &operator*=( mat3_t<U> const& m ) {
		return (*this = *this * m);
	}

	template<typename U>
	mat3_t &operator/=( U const& s) {
		value[0] /= s;
		value[1] /= s;
		value[2] /= s;
		return *this;
	}

	template<typename U>
	mat3_t &operator/=( const mat3_t<U> &m ) {
		return *this *= inverse(m);
	}

};

// Unary -
template<typename T>
inline mat3_t<T> operator-( const mat3_t<T> &m ) {
	return mat3_t<T>{ -m[0], -m[1], -m[2] };
}

// -- Binary operators --
template<typename T>
inline mat3_t<T> operator+( const mat3_t<T> &m, const T &s ) {
	return mat3_t<T>{ m[0] + s, m[1] + s, m[2] + s };
}

template<typename T>
inline mat3_t<T> operator+( const T &s, const mat3_t<T> &m ) {
	return m + s;
}

template<typename T>
inline mat3_t<T> operator+( const mat3_t<T> &m1, const mat3_t<T> &m2 ) {
	return mat3_t<T>{ m1[0] + m2[0],
					 m1[1] + m2[1],
					 m1[2] + m2[2] };
}

template<typename T>
inline mat3_t<T> operator-( const mat3_t<T> &m, const T &s ) {
	return mat3_t<T>{ m[0] - s, m[1] - s, m[2] - s };
}


template<typename T>
inline mat3_t<T> operator-( const T &s, const mat3_t<T> &m ) {
	return mat3_t<T>{ s - m[0], s - m[1], s - m[2] };
}

template<typename T>
inline mat3_t<T> operator-( const mat3_t<T> &m1, const mat3_t<T> &m2) {
	return mat3_t<T>{ m1[0] - m2[0],
					 m1[1] - m2[1],
					 m1[2] - m2[2] };
}

template<typename T>
inline mat3_t<T> operator*( const mat3_t<T> &m, const T &s ) {
	return mat3_t<T>{ m[0] * s, m[1] * s, m[2] * s };
}

template<typename T>
inline mat3_t<T> operator*( const T &s, const mat3_t<T> &m ) {
	m * s;
}

// @todo look that up: Multiplication/division with row vector returns column vector
template<typename T>
inline vec3_t<T> operator*( const mat3_t<T> &m, const vec3_t<T> &v /* rowV */ ) {
	return vec3_t<T>{ m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
					 m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
					 m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z };
}

// Multiplication with column vector returns row vector
template<typename T>
inline vec3_t<T> operator*( const vec3_t<T> &v /* colV */, const mat3_t<T> &m ) {
	return vec3_t<T>{ m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
					 m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
					 m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z };
}

template<typename T>
inline mat3_t<T> operator*( const mat3_t<T> &m1, const mat3_t<T> &m2 ) {
	T const SrcA00 = m1[0][0];
	T const SrcA01 = m1[0][1];
	T const SrcA02 = m1[0][2];
	T const SrcA10 = m1[1][0];
	T const SrcA11 = m1[1][1];
	T const SrcA12 = m1[1][2];
	T const SrcA20 = m1[2][0];
	T const SrcA21 = m1[2][1];
	T const SrcA22 = m1[2][2];
	T const SrcB00 = m2[0][0];
	T const SrcB01 = m2[0][1];
	T const SrcB02 = m2[0][2];
	T const SrcB10 = m2[1][0];
	T const SrcB11 = m2[1][1];
	T const SrcB12 = m2[1][2];
	T const SrcB20 = m2[2][0];
	T const SrcB21 = m2[2][1];
	T const SrcB22 = m2[2][2];
	// 3 vectors @todo check this !
	mat3_t<T> result;
	result[0][0] = SrcA00 * SrcB00 + SrcA10 * SrcB01 + SrcA20 * SrcB02;
	result[0][1] = SrcA01 * SrcB00 + SrcA11 * SrcB01 + SrcA21 * SrcB02;
	result[0][2] = SrcA02 * SrcB00 + SrcA12 * SrcB01 + SrcA22 * SrcB02;
	result[1][0] = SrcA00 * SrcB10 + SrcA10 * SrcB11 + SrcA20 * SrcB12;
	result[1][1] = SrcA01 * SrcB10 + SrcA11 * SrcB11 + SrcA21 * SrcB12;
	result[1][2] = SrcA02 * SrcB10 + SrcA12 * SrcB11 + SrcA22 * SrcB12;
	result[2][0] = SrcA00 * SrcB20 + SrcA10 * SrcB21 + SrcA20 * SrcB22;
	result[2][1] = SrcA01 * SrcB20 + SrcA11 * SrcB21 + SrcA21 * SrcB22;
	result[2][2] = SrcA02 * SrcB20 + SrcA12 * SrcB21 + SrcA22 * SrcB22;
	return result;
}

template<typename T>
inline mat3_t<T> operator/( const mat3_t<T> &m, const T &s) {
	return mat3_t<T>{ m[0] / s, m[1] / s, m[2] / s };
}

template<typename T>
inline mat3_t<T> operator/( const T &s, const mat3_t<T> &m ) {
	return mat3_t<T>{ s / m[0], s / m[1], s / m[2] };
}

// @todo look that up: Multiplication/division with row vector returns column vector
template<typename T>
inline vec3_t<T> operator/( const mat3_t<T> &m, const vec3_t<T> &v ) {
	return inverse(m) * v;
}

template<typename T>
inline vec3_t<T> operator/( const vec3_t<T> &v, const mat3_t<T> &m ) {
	return v * inverse(m);
}

template<typename T>
inline mat3_t<T> operator/( const mat3_t<T> &m1, const mat3_t<T> &m2 ) {
	mat3_t<T> m1_copy{m1};
	return m1_copy /= m2;
}

// -- Boolean operators --
template<typename T>
inline bool operator==( const mat3_t<T> &m1, const mat3_t<T> &m2 ) {
	return (m1[0] == m2[0]) && (m1[1] == m2[1]) && (m1[2] == m2[2]);
}

template<typename T>
inline bool operator!=( const mat3_t<T> &m1, const mat3_t<T> &m2 ) {
	return (m1[0] != m2[0]) || (m1[1] != m2[1]) || (m1[2] != m2[2]);
}

template<typename T>
inline static mat3_t<T> inverse( const mat3_t<T> &m ) {
	const T OneOverDeterminant = static_cast<T>(1) / (
				+ m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
				- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
				+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]) );
	mat3_t<T> inv;
	inv[0][0] = + (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * OneOverDeterminant;
	inv[1][0] = - (m[1][0] * m[2][2] - m[2][0] * m[1][2]) * OneOverDeterminant;
	inv[2][0] = + (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * OneOverDeterminant;
	inv[0][1] = - (m[0][1] * m[2][2] - m[2][1] * m[0][2]) * OneOverDeterminant;
	inv[1][1] = + (m[0][0] * m[2][2] - m[2][0] * m[0][2]) * OneOverDeterminant;
	inv[2][1] = - (m[0][0] * m[2][1] - m[2][0] * m[0][1]) * OneOverDeterminant;
	inv[0][2] = + (m[0][1] * m[1][2] - m[1][1] * m[0][2]) * OneOverDeterminant;
	inv[1][2] = - (m[0][0] * m[1][2] - m[1][0] * m[0][2]) * OneOverDeterminant;
	inv[2][2] = + (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * OneOverDeterminant;
	return inv;
}

// Angles in radians !
template<typename T>
inline static mat3_t<T> xRotation3x3( const T &angle) {
	T c{ std::cos( angle ) };
	T s{ std::sin( angle ) };
	return mat3_t<T>{
		T{1}, T{0}, T{0},
		T{0}, c, -s,
		T{0}, s, c
	};
}

// Angles in radians !
template<typename T>
inline static mat3_t<T> yRotation3x3( const T &angle) {
	T c{ std::cos( angle ) };
	T s{ std::sin( angle ) };
	return mat3_t<T>{
		c, T{0}, s,
		T{0}, T{1}, T{0},
		-s, T{0}, c
	};
}

// Angles in radians !
template<typename T>
inline static mat3_t<T> zRotation3x3( const T &angle) {
	T c{ std::cos( angle ) };
	T s{ std::sin( angle ) };
	return mat3_t<T>{
		c, -s, T{0},
		s, c, T{0},
		T{0}, T{0}, T{1}
	};
}

// Angles in radians !
// @todo combine to acoid multiple sin/cos calc
template<typename T>
inline mat3_t<T> rotation3x3( const T &pitchOrX, const T &yawOrY, const T &rollOrZ ) {
	return zRotation3x3(rollOrZ) * xRotation3x3(pitchOrX) * yRotation3x3(yawOrY);
}

template <typename T>
std::ostream &operator<<( std::ostream &o, mat3_t<T> const &m ) {
	o << '(' << m[0] << ',' << m[1] << ',' << m[2] << ')';
	return o;
}

typedef mat3_t<float> mat3;
typedef mat3_t<double> dmat3;

}
