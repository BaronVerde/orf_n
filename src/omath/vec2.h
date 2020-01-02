
#pragma once

#include <omath/common.h>
#include <iostream>

namespace omath {

#pragma GCC diagnostic push
// to disable "anonymous structs" warning
#pragma GCC diagnostic ignored "-Wpedantic"

template<typename T>
struct vec2_t {
	union {
		struct{ T x, y; };
		struct{ T r, g; };
		struct{ T s, t; };
	};

#pragma GCC diagnostic pop

	vec2_t() : x{0}, y{0} {}

	vec2_t( const vec2_t<T> &rhs ) = default;

	vec2_t( vec2_t<T> &&rhs ) = default;

	vec2_t &operator=( const vec2_t<T> &rhs ) = default;

	vec2_t &operator=( vec2_t<T> &&rhs ) = default;

	vec2_t( const T& scalar ) : x{scalar}, y{scalar} {}

	template<typename U>
	vec2_t( U const& s1, U const& s2 ) :
		x{static_cast<T>(s1)}, y{static_cast<T>(s2)} {}

	template<typename U>
	vec2_t( vec2_t<U> const& other ) :
		x{static_cast<T>(other.x)}, y{static_cast<T>(other.y)} {}

	// @todo bounds checking
	T &operator[]( int i ) {
		switch(i) {
			default:
			case 0:
				return x;
			case 1:
				return y;
		}
	}

	const T& operator[]( int i ) const {
		switch(i) {
			default:
			case 0:
				return x;
			case 1:
				return y;
		}
	}

	template<typename U>
	vec2_t &operator=( vec2_t<U> const &v ) {
		x = static_cast<T>(v.x);
		y = static_cast<T>(v.y);
		return *this;
	}

	template<typename U>
	vec2_t & operator+=( const U scalar ) {
		x += static_cast<T>(scalar );
		y += static_cast<T>(scalar );
		return *this;
	}

	template<typename U>
	vec2_t & operator+=( vec2_t<U> const &v ) {
		x += static_cast<T>(v.x);
		y += static_cast<T>(v.y);
		return *this;
	}

	template<typename U>
	vec2_t & operator-=(const U scalar ) {
		x -= static_cast<T>(scalar );
		y -= static_cast<T>(scalar );
		return *this;
	}

	template<typename U>
	vec2_t & operator-=( vec2_t<U> const &v ) {
		x -= static_cast<T>(v.x);
		y -= static_cast<T>(v.y);
		return *this;
	}

	template<typename U>
	vec2_t & operator*=( const U scalar ) {
		x *= static_cast<T>(scalar );
		y *= static_cast<T>(scalar );
		return *this;
	}

	template<typename U>
	vec2_t & operator*=( vec2_t<U> const &v ) {
		x *= static_cast<T>(v.x);
		y *= static_cast<T>(v.y);
		return *this;
	}

	template<typename U>
	vec2_t & operator/=(const U scalar ) {
		x /= static_cast<T>(scalar );
		y /= static_cast<T>(scalar );
		return *this;
	}

	template<typename U>
	vec2_t & operator/=( vec2_t<U> const &v ) {
		x /= static_cast<T>(v.x);
		y /= static_cast<T>(v.y);
		return *this;
	}

};

template <typename T>
inline vec2_t<T> operator-( const vec2_t<T> &v ) {
	return vec2_t<T>{ -v.x, -v.y };
}

// Binary ops
template<typename T>
inline vec2_t<T> operator+( const vec2_t<T> &v, const T scalar ) {
	return vec2_t<T>{ v.x + scalar, v.y + scalar };
}

template<typename T>
inline vec2_t<T> operator+( const T scalar, const vec2_t<T> &v ) {
	return vec2_t<T>{ scalar + v.x,	scalar + v.y };
}

template<typename T>
inline vec2_t<T> operator+( const vec2_t<T> &v1, const vec2_t<T> &v2 ) {
	return vec2_t<T>( v1.x + v2.x, v1.y + v2.y);
}

template<typename T>
inline vec2_t<T> operator-( const vec2_t<T> &v, const T scalar ) {
	return vec2_t<T>( v.x - scalar, v.y - scalar );
}

template<typename T>
inline vec2_t<T> operator-( const T scalar, const vec2_t<T> &v ) {
	return vec2_t<T>( scalar - v.x, scalar - v.y);
}

template<typename T>
inline vec2_t<T> operator-( const vec2_t<T> &v1, const vec2_t<T> &v2 ) {
	return vec2_t<T>( v1.x - v2.x, v1.y - v2.y);
}

template<typename T>
inline vec2_t<T> operator*( const vec2_t<T> &v, const T scalar ) {
	return vec2_t<T>( v.x * scalar, v.y * scalar );
}

template<typename T>
inline vec2_t<T> operator*( const T scalar, const vec2_t<T> &v ) {
	return vec2_t<T>( scalar * v.x, scalar * v.y);
}

template<typename T>
inline vec2_t<T> operator*( const vec2_t<T> &v1, const vec2_t<T> &v2 ) {
	return vec2_t<T>( v1.x * v2.x, v1.y * v2.y);
}

template<typename T>
inline vec2_t<T> operator/( const vec2_t<T> &v, const T scalar ) {
	return vec2_t<T>( v.x / scalar, v.y / scalar );
}

template<typename T>
inline vec2_t<T> operator/(T scalar, const vec2_t<T> &v ) {
	return vec2_t<T>( scalar / v.x,	scalar / v.y);
}

template<typename T>
inline vec2_t<T> operator/( const vec2_t<T> &v1, const vec2_t<T> &v2 ) {
	return vec2_t<T>( v1.x / v2.x, v1.y / v2.y);
}

// Boolean ops
template<typename T>
inline bool operator==( const vec2_t<T> &v1, const vec2_t<T> &v2 ) {
	return compareFloat( v1.x, v2.x ) && compareFloat( v1.y, v2.y );
}

template<typename T>
inline bool operator!=( const vec2_t<T> &v1, const vec2_t<T> &v2 ) {
	return !(v1 == v2 );
}

// Geometry
template <typename T>
static inline T dot( const vec2_t<T> &l, const vec2_t<T> &r ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'dot' only accepts floating-point input.\n";
	return l.x * r.x + l.y * r.y;
}

template <typename T>
static inline T magnitude( const vec2_t<T> &l ) {
	return std::sqrt( dot( l, l ) );
}

template <typename T>
static inline T magnitudeSq( const vec2_t<T> &l ) {
	return dot( l, l );
}

template <typename T>
static inline T distance( const vec2_t<T> &l, const vec2_t<T> &r ) {
	return magnitude( l - r );
}

template <typename T>
static inline T distanceSq( const vec2_t<T> &l, const vec2_t<T> &r ) {
	return magnitudeSq( l - r );
}

template <typename T>
static inline vec2_t<T> normalize( const vec2_t<T> &v ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'normalize' only accepts floating-point input.\n";
	return vec2_t<T>{ v / magnitude( v ) };
}

template <typename T>
static inline T angle( const vec2_t<T> &l, const vec2_t<T> &r ) {
	return std::acos( dot( l, r ) / std::sqrt( magnitudeSq(l) * magnitudeSq(r ) ) );
}

template <typename T>
static inline vec2_t<T> project( const vec2_t<T> &length, const vec2_t<T> &direction ) {
	return direction * ( dot( length, direction ) / magnitudeSq(direction) );
}

template <typename T>
static inline vec2_t<T> perpendicular( const vec2_t<T> &length, const vec2_t<T> &direction ) {
	return length - project( length, direction );
}

template <typename T>
static inline vec2_t<T> reflection( const vec2_t<T> &sourceVector, const vec2_t<T> &normal ) {
	return sourceVector - normal * ( dot( sourceVector, normal ) *  2.0f );
}

template <typename T>
static inline vec2_t<T> clamp( const vec2_t<T> &v, const T &minimum, const T &maximum  ) {
	return vec2_t<T>{ clamp( v.x, minimum, maximum ), clamp( v.y, minimum, maximum ) };
}

template <typename T>
std::ostream &operator<<( std::ostream &o, const vec2_t<T> &v ) {
	o << '(' << v.x << '/' << v.y << ')';
	return o;
}

typedef vec2_t<float> vec2;
typedef vec2_t<double> dvec2;
typedef vec2_t<int> ivec2;
typedef vec2_t<unsigned int> uvec2;

}
