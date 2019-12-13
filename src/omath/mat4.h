
#pragma once

#include <omath/mat3.h>
#include <omath/vec4.h>
#include <cstring>	// memcpy()
#include <array>

namespace omath {

template<typename T>
struct mat4_t {
	vec4_t<T> value[4];

	mat4_t() :
			value{ vec4_t<T>{ 1, 0, 0, 0 }, vec4_t<T>{ 0, 1, 0, 0 },
				   vec4_t<T>{ 0, 0, 1, 0 }, vec4_t<T>{ 0, 0, 0, 1 } } {}

	explicit mat4_t( const T& x ) :
			value{ vec4_t<T>{ x, 0, 0, 0 }, vec4_t<T>{ 0, x, 0, 0 },
				   vec4_t<T>{ 0, 0, x, 0 }, vec4_t<T>{ 0, 0, 0, x } } {}

	explicit mat4_t(
			const T& x0, const T& y0, const T& z0, const T& w0,
			const T& x1, const T& y1, const T& z1, const T& w1,
			const T& x2, const T& y2, const T& z2, const T& w2,
			const T& x3, const T& y3, const T& z3, const T& w3 ) :
			value{ vec4_t<T>{ x0, y0, z0, w0 }, vec4_t<T>{ x1, y1, z1, w1 },
				   vec4_t<T>{ x2, y2, z2, w2 }, vec4_t<T>{ x3, y3, z3, w3 } } {}

	mat4_t( vec4_t<T> const& v0, vec4_t<T> const& v1,
			vec4_t<T> const& v2, vec4_t<T> const& v3 ) :
				value{ v0, v1, v2, v3 } {}

	mat4_t( const mat4_t<T> &rhs ) = default;

	mat4_t( mat4_t<T> &&rhs ) = default;

	mat4_t &operator=( const mat4_t<T> &rhs ) = default;

	mat4_t &operator=( mat4_t<T> &&rhs ) = default;

	// Conversions
	template <typename U>
	mat4_t(	U const& x0, U const& y0, U const& z0, U const& w0,
			U const& x1, U const& y1, U const& z1, U const& w1,
			U const& x2, U const& y2, U const& z2, U const& w2,
			U const& x3, U const& y3, U const& z3, U const& w3 ) :
			mat4_t{ { x0, y0, z0, w0 }, { x1, y1, z1, w1 },
					{ x2, y2, z2, w2 }, { x3, y3, z3, w3 } } {}

	template<typename U>
	mat4_t( vec4_t<U> const& v0, vec4_t<U> const& v1,
			vec4_t<U> const& v2, vec4_t<U> const& v3 ) :
		value{ v0, v1, v2, v3 } {}

	// -- Matrix conversions --
	template<typename U>
	mat4_t( mat4_t<U> const& m ) :
		value{ m[0], m[1], m[2], m[3] } {}

	template<typename U>
	mat4_t( mat3_t<U> const& m ) :
		value{ vec4_t<T>{ m[0], 0 },
			   vec4_t<T>{ m[1], 0 },
			   vec4_t<T>{ m[2], 0 },
			   vec4_t<T>{ 0, 0, 0, 1 } } {}

	// Access
	vec4_t<T> &operator[]( const int& i ) {
		return value[i];
	}

	const vec4_t<T> &operator[]( const int& i ) const {
		return value[i];
	}

	// Conversions
	operator mat3_t<T>() const {
		/*return mat3_t<T> { value[0].x, value[0].y, value[0].z,
						   value[1].x, value[1].y, value[1].z,
						   value[2].x, value[2].y, value[2].z };*/
		return mat3_t<T> { value[0].x, value[1].x, value[2].x,
						   value[0].y, value[1].y, value[2].y,
						   value[0].z, value[1].z, value[2].z };
	}


	template<typename U>
	mat4_t &operator=( mat4_t<U> const& m ) {
		//std::memcpy( &value, &m.value, 16 * sizeof( U ) );
		value[0] = m[0];
		value[1] = m[1];
		value[2] = m[2];
		value[3] = m[3];
		return *this;;
	}

	template<typename U>
	mat4_t & operator+=( U const s ) {
		value[0] += s;
		value[1] += s;
		value[2] += s;
		value[3] += s;
		return *this;
	}

	template<typename U>
	mat4_t & operator+=( mat4_t<U> const& m ) {
		value[0] += m[0];
		value[1] += m[1];
		value[2] += m[2];
		value[3] += m[3];
		return *this;
	}

	template<typename U>
	mat4_t & operator-=( U const s ) {
		value[0] -= s;
		value[1] -= s;
		value[2] -= s;
		value[3] -= s;
		return *this;
	}

	template<typename U>
	mat4_t & operator-=( mat4_t<U> const& m ) {
		value[0] -= m[0];
		value[1] -= m[1];
		value[2] -= m[2];
		value[3] -= m[3];
		return *this;
	}

	template<typename U>
	mat4_t & operator*=( U const s ) {
		value[0] *= s;
		value[1] *= s;
		value[2] *= s;
		value[3] *= s;
		return *this;
	}

	template<typename U>
	mat4_t & operator*=( mat4_t<U> const& m ) {
		return (*this = *this * m);
	}

	template<typename U>
	mat4_t & operator/=( U const s ) {
		value[0] /= s;
		value[1] /= s;
		value[2] /= s;
		value[3] /= s;
		return *this;
	}

	template<typename U>
	mat4_t & operator/=( mat4_t<U> const& m ) {
		return *this *= inverse(m);
	}

};

// Unary -
template<typename T>
inline mat4_t<T> operator-( const mat4_t<T>& m ) {
	return mat4_t<T>{ -m[0], -m[1], -m[2], -m[3] };
}

// -- Binary operators --
template<typename T>
inline mat4_t<T> operator+( const mat4_t<T>& m, const T& s ) {
	return mat4_t<T>{ m[0] + s, m[1] + s, m[2] + s, m[3] + s };
}

template<typename T>
inline mat4_t<T> operator+(const T& s, const mat4_t<T>& m ) {
	return m + s;
}

template<typename T>
inline mat4_t<T> operator+( const mat4_t<T>& m1, const mat4_t<T>& m2 ) {
	return mat4_t<T>{ m1[0] + m2[0], m1[1] + m2[1], m1[2] + m2[2], m1[3] + m2[3] };
}

template<typename T>
inline mat4_t<T> operator-( const mat4_t<T>& m, const T& s ) {
	return mat4_t<T>{ m[0] - s, m[1] - s, m[2] - s, m[3] - s };
}

template<typename T>
inline mat4_t<T> operator-(const T& s, const mat4_t<T>& m ) {
	return mat4_t<T>{ s - m[0], s - m[1], s - m[2], s - m[3] };
}

template<typename T>
inline mat4_t<T> operator-( const mat4_t<T>& m1, const mat4_t<T>& m2 ) {
	return mat4_t<T>{ m1[0] - m2[0], m1[1] - m2[1], m1[2] - m2[2], m1[3] - m2[3] };
}

template<typename T>
inline mat4_t<T> operator*( const mat4_t<T>& m, const T& s ) {
	return mat4_t<T>{ m[0] * s, m[1] * s, m[2] * s, m[3] * s };
}

template<typename T>
inline mat4_t<T> operator*(const T& s, const mat4_t<T>& m ) {
	return m * s;
}

// column vctor = matrix * row vector
template<typename T>
inline vec4_t<T> operator*( const mat4_t<T>& m, vec4_t<T> const& v ) {
	/*vec4_t<T> const Mov0(v[0]);
	vec4_t<T> const Mov1(v[1]);
	vec4_t<T> const Mul0 = m[0] * Mov0;
	vec4_t<T> const Mul1 = m[1] * Mov1;
	vec4_t<T> const Add0 = Mul0 + Mul1;
	vec4_t<T> const Mov2(v[2]);
	vec4_t<T> const Mov3(v[3]);
	vec4_t<T> const Mul2 = m[2] * Mov2;
	vec4_t<T> const Mul3 = m[3] * Mov3;
	vec4_t<T> const Add1 = Mul2 + Mul3;
	vec4_t<T> const Add2 = Add0 + Add1;
	return Add2;*/
	return vec4_t<T>{ m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3],
					  m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3],
					  m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3],
					  m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3] };
}

// row vector = column vector * matrix
template<typename T>
inline vec4_t<T> operator*( vec4_t<T> const& v, const mat4_t<T>& m ) {
	return vec4_t<T>{ m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3],
					  m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3],
					  m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3],
					  m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3] };
}

template<typename T>
inline mat4_t<T> operator*( const mat4_t<T>& m1, const mat4_t<T>& m2 ) {
	vec4_t<T> const SrcA0 = m1[0];
	vec4_t<T> const SrcA1 = m1[1];
	vec4_t<T> const SrcA2 = m1[2];
	vec4_t<T> const SrcA3 = m1[3];
	vec4_t<T> const SrcB0 = m2[0];
	vec4_t<T> const SrcB1 = m2[1];
	vec4_t<T> const SrcB2 = m2[2];
	vec4_t<T> const SrcB3 = m2[3];
	mat4_t<T> result;
	result[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
	result[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
	result[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
	result[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];
	return result;
}

template<typename T>
inline mat4_t<T> operator/( const mat4_t<T>& m, const T& s ) {
	return mat4_t<T>{ m[0] / s, m[1] / s, m[2] / s, m[3] / s };
}


template<typename T>
inline mat4_t<T> operator/(const T& s, const mat4_t<T>& m ) {
	return mat4_t<T>{ s / m[0], s / m[1], s / m[2], s / m[3] };
}

// col vector = matrix * row vector
template<typename T>
inline vec4_t<T> operator/( const mat4_t<T>& m, vec4_t<T> const& v ) {
	return inverse(m) * v;
}

template<typename T>
inline vec4_t<T> operator/( vec4_t<T> const& v, const mat4_t<T>& m ) {
	return v * inverse(m);
}

template<typename T>
inline mat4_t<T> operator/( const mat4_t<T>& m1, const mat4_t<T>& m2 ) {
	mat4_t<T> m1_copy(m1);
	return m1_copy /= m2;
}

// -- Boolean operators --
template<typename T>
inline bool operator==( const mat4_t<T>& m1, const mat4_t<T>& m2 ) {
	return( m1[0] == m2[0]) && (m1[1] == m2[1]) && (m1[2] == m2[2]) && (m1[3] == m2[3] );
}

template<typename T>
inline bool operator!=( const mat4_t<T>& m1, const mat4_t<T>& m2 ) {
	return !(m1 == m2);
}

template <typename T>
std::ostream &operator<<( std::ostream &o, const mat4_t<T> &m ) {
	o << '(' << m[0] << ',' << m[1] << ',' << m[2] << ',' << m[3] << ')';
	return o;
}

template<typename T>
inline static mat4_t<T> inverse( const mat4_t<T>& m ) {
	const T coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
	const T coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
	const T coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
	const T coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
	const T coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
	const T coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
	const T coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
	const T coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
	const T coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
	const T coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
	const T coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
	const T coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
	const T coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
	const T coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
	const T coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
	const T coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
	const T coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	const T coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
	vec4_t<T> fac0( coef00, coef00, coef02, coef03 );
	vec4_t<T> fac1( coef04, coef04, coef06, coef07 );
	vec4_t<T> fac2( coef08, coef08, coef10, coef11 );
	vec4_t<T> fac3( coef12, coef12, coef14, coef15 );
	vec4_t<T> fac4( coef16, coef16, coef18, coef19 );
	vec4_t<T> fac5( coef20, coef20, coef22, coef23 );
	vec4_t<T> vec0( m[1][0], m[0][0], m[0][0], m[0][0] );
	vec4_t<T> vec1( m[1][1], m[0][1], m[0][1], m[0][1] );
	vec4_t<T> vec2( m[1][2], m[0][2], m[0][2], m[0][2] );
	vec4_t<T> vec3( m[1][3], m[0][3], m[0][3], m[0][3] );
	vec4_t<T> inv0( vec1 * fac0 - vec2 * fac1 + vec3 * fac2 );
	vec4_t<T> inv1( vec0 * fac0 - vec2 * fac3 + vec3 * fac4 );
	vec4_t<T> inv2( vec0 * fac1 - vec1 * fac3 + vec3 * fac5 );
	vec4_t<T> inv3( vec0 * fac2 - vec1 * fac4 + vec2 * fac5 );
	vec4_t<T> signA( +1, -1, +1, -1 );
	vec4_t<T> signB( -1, +1, -1, +1 );
	mat4_t<T> inv{ inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB };
	vec4_t<T> row0( inv[0][0], inv[1][0], inv[2][0], inv[3][0] );
	vec4_t<T> dot0( m[0] * row0 );
	const T dot1 = ( dot0.x + dot0.y ) + ( dot0.z + dot0.w );
	const T OneOverDeterminant = static_cast<T>(1) / dot1;
	return inv * OneOverDeterminant;
}

// All matrices right handed !
template<typename T>
inline static mat4_t<T> ortho( const T& left, const T& right, const T& bottom, const T& top ) {
	mat4_t<T> result{ static_cast<T>(1) };
	result[0][0] = static_cast<T>(2) / (right - left);
	result[1][1] = static_cast<T>(2) / (top - bottom);
	result[2][2] = - static_cast<T>(1);
	result[3][0] = - (right + left) / (right - left);
	result[3][1] = - (top + bottom) / (top - bottom);
	return result;
}

template<typename T>
inline static mat4_t<T> ortho( const T& left, const T& right, const T& bottom, const T& top,
		const T& zNear, const T& zFar ) {
	mat4_t<T> result{ static_cast<T>(1) };
	result[0][0] = static_cast<T>(2) / (right - left);
	result[1][1] = static_cast<T>(2) / (top - bottom);
	result[2][2] = - static_cast<T>(2) / (zFar - zNear);
	result[3][0] = - (right + left) / (right - left);
	result[3][1] = - (top + bottom) / (top - bottom);
	result[3][2] = - (zFar + zNear) / (zFar - zNear);
	return result;
}

template<typename T>
inline static mat4_t<T> frustum( const T& left, const T& right, const T& bottom,
		const T& top, const T& nearVal, const T& farVal ) {
	mat4_t<T> result( static_cast<T>(0) );
	result[0][0] = (static_cast<T>(2) * nearVal) / (right - left);
	result[1][1] = (static_cast<T>(2) * nearVal) / (top - bottom);
	result[2][0] = (right + left) / (right - left);
	result[2][1] = (top + bottom) / (top - bottom);
	result[2][2] = - (farVal + nearVal) / (farVal - nearVal);
	result[2][3] = static_cast<T>(-1);
	result[3][2] = - (static_cast<T>(2) * farVal * nearVal) / (farVal - nearVal);
	return result;
}

// Corresponds to glm RH_NO version !
template<typename T>
inline static mat4_t<T> perspective( const T& fovy, const T& aspect, const T& zNear, const T& zFar ) {
	const T tanHalfFovy{ std::tan( fovy / static_cast<T>(2) ) };
	mat4_t<T> result{ static_cast<T>(0) };
	result[0][0] = static_cast<T>(1) / ( aspect * tanHalfFovy );
	result[1][1] = static_cast<T>(1) / tanHalfFovy;
	result[2][2] = -( zFar + zNear ) / ( zFar - zNear );
	result[2][3] = -static_cast<T>(1);
	result[3][2] = -( static_cast<T>(2) * zFar * zNear ) / ( zFar - zNear );
	return result;
}

template<typename T>
inline static mat4_t<T> infinitePerspective( const T& fovy, const T& aspect, const T& zNear ) {
	const T range = tan(fovy / static_cast<T>(2)) * zNear;
	const T left = -range * aspect;
	const T right = range * aspect;
	const T bottom = -range;
	const T top = range;
	mat4_t<T> result{ static_cast<T>(0) };
	result[0][0] = (static_cast<T>(2) * zNear) / (right - left);
	result[1][1] = (static_cast<T>(2) * zNear) / (top - bottom);
	result[2][2] = - static_cast<T>(1);
	result[2][3] = - static_cast<T>(1);
	result[3][2] = - static_cast<T>(2) * zNear;
	return result;
}

// Infinite projection matrix: http://www.terathon.com/gdc07_lengyel.pdf
template<typename T>
inline static mat4_t<T> tweakedInfinitePerspective(
		const T& fovy, const T& aspect, const T& zNear, const T& ep ) {
	const T range = tan(fovy / static_cast<T>(2)) * zNear;
	const T left = -range * aspect;
	const T right = range * aspect;
	const T bottom = -range;
	const T top = range;
	mat4_t<T> result{ static_cast<T>(0) };
	result[0][0] = (static_cast<T>(2) * zNear) / (right - left);
	result[1][1] = (static_cast<T>(2) * zNear) / (top - bottom);
	result[2][2] = ep - static_cast<T>(1);
	result[2][3] = static_cast<T>(-1);
	result[3][2] = (ep - static_cast<T>(2)) * zNear;
	return result;
}

// Right handed
template<typename T>
inline static mat4_t<T> lookAt( const vec3_t<T>& eye, const vec3_t<T>& center, const vec3_t<T>& up ) {
	const vec3_t<T> f{ normalize( center - eye ) };
	const vec3_t<T> s{ normalize( cross( f, up ) ) };
	const vec3_t<T> u{ cross( s, f ) };
	mat4_t<T> result{ static_cast<T>(1) };
	result[0][0] = s.x;
	result[1][0] = s.y;
	result[2][0] = s.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] =-f.x;
	result[1][2] =-f.y;
	result[2][2] =-f.z;
	result[3][0] =-dot( s, eye );
	result[3][1] =-dot( u, eye );
	result[3][2] = dot( f, eye );
	return result;
}

template<typename T>
inline static mat4_t<T> translate( const mat4_t<T>& m, const vec3_t<T>& v ) {
		 mat4_t<T> result(m);
	result[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
	return result;
}

template<typename T>
inline static mat4_t<T> rotate( const mat4_t<T>& m, T angle, const vec3_t<T>& v ) {
	const T a = angle;
	const T c = std::cos(a);
	const T s = std::sin(a);
	vec3_t<T> axis(normalize(v));
	vec3_t<T> temp((T(1) - c) * axis);
	mat4_t<T> rotM;
	rotM[0][0] = c + temp[0] * axis[0];
	rotM[0][1] = temp[0] * axis[1] + s * axis[2];
	rotM[0][2] = temp[0] * axis[2] - s * axis[1];
	rotM[1][0] = temp[1] * axis[0] - s * axis[2];
	rotM[1][1] = c + temp[1] * axis[1];
	rotM[1][2] = temp[1] * axis[2] + s * axis[0];
	rotM[2][0] = temp[2] * axis[0] + s * axis[1];
	rotM[2][1] = temp[2] * axis[1] - s * axis[0];
	rotM[2][2] = c + temp[2] * axis[2];
	mat4_t<T> result;
	result[0] = m[0] * rotM[0][0] + m[1] * rotM[0][1] + m[2] * rotM[0][2];
	result[1] = m[0] * rotM[1][0] + m[1] * rotM[1][1] + m[2] * rotM[1][2];
	result[2] = m[0] * rotM[2][0] + m[1] * rotM[2][1] + m[2] * rotM[2][2];
	result[3] = m[3];
	return result;
}

template<typename T>
inline static mat4_t<T> scale( const mat4_t<T>& m, const vec3_t<T>& v ) {
	mat4_t<T> result;
	result[0] = m[0] * v[0];
	result[1] = m[1] * v[1];
	result[2] = m[2] * v[2];
	result[3] = m[3];
	return result;
}

typedef mat4_t<float> mat4;
typedef mat4_t<double> dmat4;

}
