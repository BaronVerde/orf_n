
#pragma once

#include <omath/vec2.h>

namespace omath {

#pragma GCC diagnostic push
// to disable "anonymous structs" warning
#pragma GCC diagnostic ignored "-Wpedantic"

template<typename T>
struct vec3_t {
	union {
		struct{ T x, y, z; };
		struct{ T r, g, b; };
		struct{ T s, t, u; };
	};

#pragma GCC diagnostic pop

	vec3_t() : x{0}, y{0}, z{0} {}

	vec3_t( const vec3_t<T> &rhs ) = default;

	vec3_t( vec3_t<T> &&rhs ) = default;

	vec3_t &operator=( const vec3_t<T> &rhs ) = default;

	vec3_t &operator=( vec3_t<T> &&rhs ) = default;

	vec3_t( const T &scalar ) :
			x{scalar}, y{scalar}, z{scalar} {}

	vec3_t( const T &s1, const T &s2, const T &s3 ) :
			x{s1}, y{s2}, z{s3} {}

	vec3_t( const vec2_t<T> &v, const T &s3 ) :
			x{v.x}, y{v.y}, z{s3} {}

	vec3_t( const T &s1, const vec2_t<T> &v ) :
			x{s1}, y{v.x}, z{v.z} {}

	template<typename U>
	vec3_t<T>( const vec3_t<U> &other ) :
		x{static_cast<T>(other.x)}, y{static_cast<T>(other.y)}, z{static_cast<T>(other.z)} {}

	// @todo bounds checking !
	T &operator[]( int i ) {
		switch(i) {
			default:
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
		}
	}

	const T &operator[]( int i ) const {
		switch(i) {
			default:
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
		}
	}

	template<typename U>
	vec3_t &operator=( vec3_t<U> const &v) {
		x = static_cast<T>(v.x);
		y = static_cast<T>(v.y);
		z = static_cast<T>(v.z);
		return *this;
	}

	template<typename U>
	vec3_t & operator+=( U const scalar) {
		x += static_cast<T>(scalar);
		y += static_cast<T>(scalar);
		z += static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	vec3_t & operator+=( vec3_t<U> const &v) {
		x += static_cast<T>(v.x);
		y += static_cast<T>(v.y);
		z += static_cast<T>(v.z);
		return *this;
	}

	template<typename U>
	vec3_t & operator-=( U const scalar) {
		x -= static_cast<T>(scalar);
		y -= static_cast<T>(scalar);
		z -= static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	vec3_t & operator-=( vec3_t<U> const &v) {
		x -= static_cast<T>(v.x);
		y -= static_cast<T>(v.y);
		z -= static_cast<T>(v.z);
		return *this;
	}

	template<typename U>
	vec3_t & operator*=( U const scalar) {
		x *= static_cast<T>(scalar);
		y *= static_cast<T>(scalar);
		z *= static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	vec3_t & operator*=( vec3_t<U> const &v) {
		x *= static_cast<T>(v.x);
		y *= static_cast<T>(v.y);
		z *= static_cast<T>(v.z);
		return *this;
	}

	template<typename U>
	vec3_t & operator/=( U const scalar) {
		x /= static_cast<T>(scalar);
		y /= static_cast<T>(scalar);
		z /= static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	vec3_t & operator/=( vec3_t<U> const &v) {
		x /= static_cast<T>(v.x);
		y /= static_cast<T>(v.y);
		z /= static_cast<T>(v.z);
		return *this;
	}

};

template <typename T>
inline vec3_t<T> operator-( const vec3_t<T> &v ) {
	return vec3_t<T>{ -v.x, -v.y, -v.z };
}

// Binary ops
template<typename T>
inline vec3_t<T> operator+( const vec3_t<T> &v, const T scalar) {
	return vec3_t<T>{ v.x + scalar, v.y + scalar, v.z + scalar };
}

template<typename T>
inline vec3_t<T> operator+( const T scalar, const vec3_t<T> &v) {
	return vec3_t<T>{ scalar + v.x, scalar + v.y, scalar + v.z };
}

template<typename T>
inline vec3_t<T> operator+( const vec3_t<T> &v1, const vec3_t<T> &v2) {
	return vec3_t<T>( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
}

template<typename T>
inline vec3_t<T> operator-( const vec3_t<T> &v, const T scalar) {
	return vec3_t<T>( v.x - scalar, v.y - scalar, v.z - scalar );
}

template<typename T>
inline vec3_t<T> operator-( const T scalar, const vec3_t<T> &v) {
	return vec3_t<T>( scalar - v.x, scalar - v.y, v.z - scalar );
}

template<typename T>
inline vec3_t<T> operator-( const vec3_t<T> &v1, const vec3_t<T> &v2) {
	return vec3_t<T>( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z );
}

template<typename T>
inline vec3_t<T> operator*( const vec3_t<T> &v, const T scalar) {
	return vec3_t<T>( v.x * scalar, v.y * scalar, v.z * scalar );
}

template<typename T>
inline vec3_t<T> operator*( const T scalar, const vec3_t<T> &v) {
	return vec3_t<T>( scalar * v.x, scalar * v.y, scalar * v.z);
}

template<typename T>
inline vec3_t<T> operator*( const vec3_t<T> &v1, const vec3_t<T> &v2) {
	return vec3_t<T>( v1.x * v2.x, v1.y * v2.y, v1.z * v2.z );
}

template<typename T>
inline vec3_t<T> operator/( const vec3_t<T> &v, const T scalar) {
	return vec3_t<T>( v.x / scalar, v.y / scalar, v.z / scalar);
}

template<typename T>
inline vec3_t<T> operator/( const T scalar, const vec3_t<T> &v) {
	return vec3_t<T>( scalar / v.x, scalar / v.y, scalar / v.z );
}

template<typename T>
inline vec3_t<T> operator/( const vec3_t<T> &v1, const vec3_t<T> &v2) {
	return vec3_t<T>( v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

// Boolean ops
template<typename T>
inline bool operator==( const vec3_t<T> &v1, const vec3_t<T> &v2) {
	return compare_float( v1.x, v2.x ) && compare_float( v1.y, v2.y ) && compare_float( v1.z, v2.z );
}

template<typename T>
inline bool operator!=( const vec3_t<T> &v1, const vec3_t<T> &v2) {
	return !(v1 == v2);
}

// Geometry
template <typename T>
static inline T dot( const vec3_t<T> &l, const vec3_t<T> &r ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'dot' only accepts floating-point input.\n";
	return l.x * r.x + l.y * r.y + l.z * r.z;
}

template <typename T>
static inline vec3_t<T> cross( const vec3_t<T> &l, const vec3_t<T> &r ) {
	return vec3_t<T> { l.y * r.z - l.z * r.y,
					 l.z * r.x - l.x * r.z,
					 l.x * r.y - l.y * r.x };
}

template <typename T>
static inline T magnitude( const vec3_t<T> &l ) {
	return std::sqrt( dot( l, l ) );
}

template <typename T>
static inline T magnitudeSq( const vec3_t<T> &l ) {
	return dot( l, l );
}

template <typename T>
static inline T distance( const vec3_t<T> &l, const vec3_t<T> &r ) {
	return magnitude( l - r );
}

template <typename T>
static inline T distanceSq( const vec3_t<T> &l, const vec3_t<T> &r ) {
	return magnitudeSq( l - r );
}

template <typename T>
static inline vec3_t<T> normalize( const vec3_t<T> &v ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'normalize' only accepts floating-point input.\n";
	return vec3_t<T>{ v / magnitude( v ) };
}

template <typename T>
static inline T angle( const vec3_t<T> &l, const vec3_t<T> &r ) {
	return std::acos( dot( l, r ) / std::sqrt( magnitudeSq(l) * magnitudeSq(r) ) );
}

template <typename T>
static inline vec3_t<T> project( const vec3_t<T> &length, const vec3_t<T> &direction ) {
	return direction * ( dot( length, direction ) / magnitudeSq(direction) );
}

template <typename T>
static inline vec3_t<T> perpendicular( const vec3_t<T> &length, const vec3_t<T> &direction ) {
	return length - project( length, direction );
}

template <typename T>
static inline vec3_t<T> reflection( const vec3_t<T> &sourceVector, const vec3_t<T> &normal ) {
	return sourceVector - normal * ( dot( sourceVector, normal ) * 2.0f );
}

template <typename T>
std::ostream &operator<<( std::ostream &o, const vec3_t<T> &v ) {
	o << '(' << v.x << '/' << v.y << '/' << v.z << ')';
	return o;
}

typedef vec3_t<float> vec3;
typedef vec3_t<double> dvec3;
typedef vec3_t<int> ivec3;
typedef vec3_t<unsigned int> uvec3;

static inline void doubleToTwoFloats( const omath::dvec3 &d, omath::vec3 &high, omath::vec3 &low ) {
	high.x = static_cast<float>( d.x );
	high.y = static_cast<float>( d.y );
	high.z = static_cast<float>( d.z );
	low.x = static_cast<float>( (d.x - static_cast<double>(high.x)) );
	low.y = static_cast<float>( (d.y - static_cast<double>(high.y)) );
	low.z = static_cast<float>( (d.z - static_cast<double>(high.z)) );
}

/* Calculate position rte from world position parts high/low and deduct camera position high/low.
 * Moves the world position into camera space.
 * Takes in the position's high and low part and the camera position's high andf low part.
 * Result is reunited single precision float vector camPos - pos */
static inline vec3 calculatePositionRTE(
		const omath::vec3 &posHigh, const omath::vec3 &posLow,
		const omath::vec3 &camPosHigh, const omath::vec3 &camPosLow ) {
	vec3 t1 = posLow - camPosLow;
	vec3 e = t1 - posLow;
	vec3 t2 = ( ( -camPosLow - e ) + ( posLow - ( t1 - e ) ) ) + posHigh - camPosHigh;
	vec3 highDifference = t1 + t2;
	vec3 lowDifference = t2 - ( highDifference - t1 );
	return highDifference + lowDifference;
}

template<typename T>
static inline bool compareFloat( const omath::vec3_t<T> &one, const omath::vec3_t<T> &other ) {
	return( compareFloat( one.x, other.x ) && compareFloat( one.y, other.y ) && compareFloat( one.z, other.z ) );
}

}	// namespace omath
