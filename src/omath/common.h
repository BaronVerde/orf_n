
// @todo: use a flag to check for nans in constructors !

#pragma once

#include <limits>
#include <cmath>
#include <iostream>

namespace omath {

/**
 * Trigonometric constexprants. Mind definitions in shaders !
 */
// Square root of 3.
const double SQRT2{ 1.4142135623730950488 };
const double SQRT3{ 1.7320508075688772935 };
const double ONE_OVER_PI = 1.0 / M_PI;
const double ONE_OVER_TWO_PI = 1.0 / ( 2.0 * M_PI );
const double ONE_OVER_SQRT2{ 1.0 / std::sqrt( 2.0 ) };
const double PI_OVER_TWO = M_PI * 0.5;
const double PI_OVER_THREE = M_PI / 3.0;
const double PI_OVER_FOUR = M_PI / 4.0;
const double PI_OVER_SIX = M_PI / 6.0;
const double THREE_PI_OVER_TWO = (3.0 * M_PI) * 0.5;
const double TWO_PI = 2.0 * M_PI;

// returns v0 when t == 0 and v1 when t == 1
template<typename T>
T lerp( T v0, T v1, T t ) {
	return( ( static_cast<T>(1) - t ) * v0 + t * v1 );
}

/**
 * Interpolates between n1 and n2 with given pre-n1 value n0 and post-n1 value n3
 * If a is 0, function returns n1, if a is 1.0, function returns n2
 * After libnoise !
 */
template <typename T>
T cubic_interpolate( const T n0, const T n1, const T n2, const T n3, const T a = 0.5 ) {
	const T p{ ( n3 - n2 ) - ( n0 - n1 ) };
	const T q{ ( n0 - n1 ) - p };
	const T r{ n2 - n0 };
	const T s{ n1 };
	return p * a * a * a + q * a * a + r * a + s;
}

template<typename T>
bool is_power_of_2( T val ) {
	if( val < T(1) )
		return false;
	return( val & ( val - T(1) ) ) == 0;
}

template<typename T>
T clamp( const T &number, const T &minimum, const T &maximum ) {
	T clamped{ ( number < minimum ) ? minimum : ( number > maximum ) ? maximum : number };
	return clamped;
}

/* Compares two float numbers combining absolute and relative tolerance.
 * http://www.realtimecollisiondetection.net/pubs/Tolerances/ */
template <typename T>
bool compare_float( const T &a, const T &b, const T maxDiff, const T maxRelDiff ) {
	if( !std::numeric_limits<T>::is_iec559 ) {
		std::cerr << "Only floating point numbers can be float-compared.\n";
		return false;
	}
	const T diff{ std::fabs( a - b ) };
	if( diff <= maxDiff )
		return true;
	const T aa{ std::fabs(a) };
	const T ab{ std::fabs(b) };
	const T largest{ ( ab > aa ) ? ab : aa };
	if( diff <= largest * maxRelDiff )
		return true;
	return false;
}

// radians
template<typename T>
T radians( const T degrees ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'radians' only accepts floating-point input.\n";
	return degrees * static_cast<T>( 0.01745329251994329576923690768489 );
}

// degrees
template<typename T>
T degrees( const T radians ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'degrees' only accepts floating-point input.\n";
	return radians * static_cast<T>( 57.295779513082320876798154814105 );
}

template<typename T>
T modulo_float( T const &a, T const &b ) {
	if( !std::numeric_limits<T>::is_iec559 )
		std::cerr << "'floatModulo' only accepts floating-point input.\n";
	return a - b * floor(a / b);
}

template <typename T>
static inline bool compare_float( const T x, const T y ) {
	if( !std::numeric_limits<T>::is_iec559 ) {
		std::cerr << "Only floating point numbers can be float-compared.\n";
		return false;
	}
	return std::fabs( x - y ) <= std::numeric_limits<T>::epsilon() *
		   std::max( static_cast<T>( 1 ), std::max( std::fabs( x ), std::fabs( y ) ) );
}

template<typename T>
static inline int signum( T val ) {
	return (T(0) < val) - (val < T(0));
}

static inline void double_to_two_floats( const double &d, float &high, float &low ) {
	high = static_cast<float>(d);
	low = static_cast<float>(d - static_cast<double>(high));
}

}
