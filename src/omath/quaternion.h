
#pragma once

#include <omath/mat4.h>

namespace omath {

template<typename T>
struct quaternion_t {

#pragma GCC diagnostic push
// to disable "anonymous structs" warning
#pragma GCC diagnostic ignored "-Wpedantic"

	 union {
		 struct {
			 T r;
			 vec3_t<T> v;
		 };
		 struct {
			 T x;
			 T y;
			 T z;
			 T w;
		 };
		 T a[4];
	 };

#pragma GCC diagnostic pop

	 quaternion_t() : r{0}, v{ T{0}, T{0}, T{0} } {}

	 quaternion_t( const quaternion_t<T> &rhs ) = default;

	 quaternion_t( quaternion_t<T> &&rhs ) = default;

	 quaternion_t &operator=( const quaternion_t<T> &rhs ) = default;

	 quaternion_t &operator=( quaternion_t<T> &&rhs ) = default;

	 quaternion_t( T _r ) : r{_r}, v{T{0}} {}

	 quaternion_t( T _r, const vec3_t<T> & _v ) : r{_r}, v{_v} {}

	 quaternion_t( const vec4_t<T> &_v ) : r{_v[0]}, v{ _v[1], _v[2], _v[3] } {}

	 quaternion_t( T _x, T _y, T _z, T _w ) : r{_x}, v{ _y, _z, _w } {}

	 T &operator[]( int n ) {
		 return a[n];
	 }

	 const T &operator[]( int n ) const {
		 return a[n];
	 }

	 quaternion_t operator+( const quaternion_t<T> &q ) const {
		 return quaternion_t<T>{ r + q.r, v + q.v };
	 }

	 quaternion_t &operator+=( const quaternion_t<T> &q ) {
		 r += q.r;
		 v += q.v;
		 return *this;
	 }

	 quaternion_t operator-( const quaternion_t &q ) const {
		 return quaternion_t<T>{ r - q.r, v - q.v };
	 }

	 quaternion_t &operator-=( const quaternion_t<T> &q ) {
		 r -= q.r;
		 v -= q.v;
		 return *this;
	 }

	 quaternion_t operator-() const {
		 return quaternion_t<T>{ -r, -v };
	 }

	 quaternion_t operator*( const T s ) const {
		 return quaternion_t<T>{ a[0] * s, a[1] * s, a[2] * s, a[3] * s };
	 }

	 quaternion_t &operator*=( const T s ) {
		 r *= s;
		 v *= s;
		 return *this;
	 }

	 quaternion_t operator*( const quaternion_t& q ) const {
		 const T x1 = a[0];
		 const T y1 = a[1];
		 const T z1 = a[2];
		 const T w1 = a[3];
		 const T x2 = q.a[0];
		 const T y2 = q.a[1];
		 const T z2 = q.a[2];
		 const T w2 = q.a[3];
		 return quaternion_t<T>{ w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
			 	 	 	 	 	 w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2,
								 w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2,
								 w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2 };
	 }

	 quaternion_t operator/( const T s ) const {
		 return quaternion_t<T>{ a[0] / s, a[1] / s, a[2] / s, a[3] / s };
	 }

	 quaternion_t &operator/=( const T s ) {
		 r /= s;
		 v /= s;
		 return *this;
	 }

	 operator vec4_t<T> &() {
		 return *(vec4_t<T>*)&a[0];
	 }

	 operator const vec4_t<T> &() const {
		 return *(const vec4_t<T>*)&a[0];
	 }

	 bool operator==( const quaternion_t<T> &q ) const {
		 return( r == q.r ) && ( v == q.v );
	 }

	 bool operator!=( const quaternion_t<T> &q ) const {
		 return( r != q.r ) || ( v != q.v );
	 }

	 mat4_t<T> asMatrix() const {
		 mat4_t<T> m;
		 const T xx = x * x;
		 const T yy = y * y;
		 const T zz = z * z;
		 const T ww = w * w;
		 const T xy = x * y;
		 const T xz = x * z;
		 const T xw = x * w;
		 const T yz = y * z;
		 const T yw = y * w;
		 const T zw = z * w;
		 m[0][0] = T(1) - T(2) * (yy + zz);
		 m[0][1] =        T(2) * (xy - zw);
		 m[0][2] =        T(2) * (xz + yw);
		 m[0][3] =        T(0);
		 m[1][0] =        T(2) * (xy + zw);
		 m[1][1] = T(1) - T(2) * (xx + zz);
		 m[1][2] =        T(2) * (yz - xw);
		 m[1][3] =        T(0);
		 m[2][0] =        T(2) * (xz - yw);
		 m[2][1] =        T(2) * (yz + xw);
		 m[2][2] = T(1) - T(2) * (xx + yy);
		 m[2][3] =        T(0);
		 m[3][0] =        T(0);
		 m[3][1] =        T(0);
		 m[3][2] =        T(0);
		 m[3][3] =        T(1);
		 return m;
	 }

	 /*
	 inline T length() const {
		return magnitude( vec4_t<T>{ r, v } );
	 }
	  */

};

typedef quaternion_t<float> quaternion;
typedef quaternion_t<double> dquaternion;

template <typename T>
static inline quaternion_t<T> operator*( T a, const quaternion_t<T> &b ) {
	return b * a;
}

template <typename T>
static inline quaternion_t<T> operator/( T a, const quaternion_t<T> &b ) {
	return quaternion_t<T>{ a / b[0], a / b[1], a / b[2], a / b[3] };
}

template <typename T>
static inline quaternion_t<T> normalize( const quaternion_t<T> &q ) {
	return q / magnitude( vec4_t<T>{q} );
}

template <typename T>
static inline void quaternionToMatrix( const quaternion_t<T> &q, mat4_t<T> &m ) {
	m = q.asMatrix();
}

}
