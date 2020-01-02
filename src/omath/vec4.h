
#pragma once

#include <omath/vec3.h>

namespace omath {

#pragma GCC diagnostic push
// to disable "anonymous structs" warning
#pragma GCC diagnostic ignored "-Wpedantic"

template<typename T>

struct vec4_t {
	union {
		struct{ T x, y, z, w; };
		struct{ T r, g, b, a; };
		struct{ T s, t, u, v; };
	};

#pragma GCC diagnostic pop

	vec4_t() : x{0}, y{0}, z{0}, w{0} {}

	vec4_t( const vec4_t<T> &rhs ) = default;

	vec4_t( vec4_t<T> &&rhs ) = default;

	vec4_t &operator=( const vec4_t<T> &rhs ) = default;

	vec4_t &operator=( vec4_t<T> &&rhs ) = default;

	explicit vec4_t( const T &scalar ) :
			x{scalar}, y{scalar}, z{scalar}, w{scalar} {}

	explicit vec4_t( const T &s1, const T &s2, const T &s3, const T &s4 ) :
			x{s1}, y{s2}, z{s3}, w{s4} {}

	vec4_t( const vec3_t<T> &v, const T &s4 ) :
			x{v.x}, y{v.y}, z{v.z}, w{s4} {}

	vec4_t( const vec3_t<T> &v ) :
			x{v.x}, y{v.y}, z{v.z}, w{1} {}

	vec4_t( const T &s1, const vec3_t<T> &v ) :
			x{s1}, y{v.x}, z{v.z}, w{v.z} {}

	template<typename U>
	vec4_t( const vec4_t<U> &other ) :
		x{static_cast<T>(other.x)}, y{static_cast<T>(other.y)},
		z{static_cast<T>(other.z)}, w{static_cast<T>(other.w)} {}

	// @todo bounds checking !
	T &operator[]( const int i ) {
		switch(i) {
			default:
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
		}
	}

	const T& operator[]( const int i ) const {
		switch(i) {
			default:
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
		}
	}

	template<typename U>
	vec4_t &operator=( vec4_t<U> const& v) {
		x = static_cast<T>(v.x);
		y = static_cast<T>(v.y);
		z = static_cast<T>(v.z);
		w = static_cast<T>(v.w);
		return *this;
	}

	template<typename U>
	vec4_t & operator*=( U const scalar) {
		x *= static_cast<T>(scalar);
		y *= static_cast<T>(scalar);
		z *= static_cast<T>(scalar);
		w *= static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	vec4_t & operator*=( vec4_t<U> const& v) {
		x *= static_cast<T>(v.x);
		y *= static_cast<T>(v.y);
		z *= static_cast<T>(v.z);
		w *= static_cast<T>(v.w);
		return *this;
	}

	template<typename U>
	vec4_t & operator/=( U const scalar) {
		x /= static_cast<T>(scalar);
		y /= static_cast<T>(scalar);
		z /= static_cast<T>(scalar);
		w /= static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	vec4_t & operator/=( vec4_t<U> const& v) {
		x /= static_cast<T>(v.x);
		y /= static_cast<T>(v.y);
		z /= static_cast<T>(v.z);
		w /= static_cast<T>(v.w);
		return *this;
	}

};

template <typename T>
inline vec4_t<T> operator-( const vec4_t<T> &v ) {
	return vec4_t<T>{ -v.x, -v.y, -v.z, -v.w };
}

template <typename T>
inline vec4_t<T> operator-( const vec4_t<T> &v1, const vec4_t<T> &v2  ) {
	return vec4_t<T>( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w );
}

template <typename T>
inline vec4_t<T> operator+( const vec4_t<T> &v1, const vec4_t<T> &v2  ) {
	return vec4_t<T>( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w );
}

template<typename T>
inline vec4_t<T> operator*( const vec4_t<T> &v, const T scalar ) {
	return vec4_t<T>( v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar );
}

template<typename T>
inline vec4_t<T> operator*( const T scalar, const vec4_t<T> &v ) {
	return vec4_t<T>( scalar * v.x, scalar * v.y, scalar * v.z, scalar * v.w );
}

template<typename T>
inline vec4_t<T> operator*( const vec4_t<T> &v1, const vec4_t<T> &v2 ) {
	return vec4_t<T>( v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w );
}

template<typename T>
inline vec4_t<T> operator/(const vec4_t<T> &v, const T scalar) {
	return vec4_t<T>( v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar );
}

template<typename T>
inline vec4_t<T> operator/( const T scalar, const vec4_t<T> &v) {
	return vec4_t<T>( scalar / v.x, scalar / v.y, scalar / v.z, scalar / v.w );
}

template<typename T>
inline vec4_t<T> operator/(const vec4_t<T> &v1, const vec4_t<T> &v2) {
	return vec4_t<T>( v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w );
}

// Boolean ops
template<typename T>
inline bool operator==(const vec4_t<T> &v1, const vec4_t<T> &v2) {
	return compareFloat( v1.x, v2.x ) && compareFloat( v1.y, v2.y ) &&
			compareFloat( v1.z, v2.z ) && compareFloat( v1.w, v2.w );
}

template<typename T>
inline bool operator!=(const vec4_t<T> &v1, const vec4_t<T> &v2) {
	return !(v1 == v2);
}

template <typename T>
static inline T dot( const vec4_t<T> &l, const vec4_t<T> &r ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'dot' only accepts floating-point input.\n";
	return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
}

template <typename T>
std::ostream &operator<<( std::ostream &o, const vec4_t<T> &v ) {
	o << '(' << v.x << '/' << v.y << '/' << v.z << '/' << v.w << ')';
	return o;
}

typedef vec4_t<float> vec4;
typedef vec4_t<double> dvec4;
typedef vec4_t<int> ivec4;
typedef vec4_t<unsigned int> uvec4;

}
