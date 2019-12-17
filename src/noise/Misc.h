
#pragma once

namespace noise {

typedef enum {
    /* When a coherent-noise function with this quality setting is used to generate a
     * bump-map image, there are noticeable "creasing" artifacts in the resulting image
     * because the derivative of that function is discontinuous at integer boundaries. */
    QUALITY_FAST = 0,
	/* When a coherent-noise function with this quality setting is used to generate a
	 * bump-map image, there are some "creasing" artifacts in the resulting image because
	 * the second derivative of that function is discontinuous at integer boundaries. */
    QUALITY_STD = 1,
	/* When a coherent-noise function with this quality setting is used to generate a
	 * bump-map image, there are no "creasing" artifacts in the resulting image because
	 * the second derivative of that function is continuous at integer boundaries. */
	QUALITY_BEST = 2
} noiseQuality_t;

/// Modifies a floating-point value so that it can be stored in a noise::int32 variable.
///
/// @param n A floating-point number.
///
/// @returns The modified floating-point number.
///
/// This function does not modify @a n.
///
/// In libnoise, the noise-generating algorithms are all integer-based;
/// they use variables of type noise::int32.  Before calling a noise
/// function, pass the @a x, @a y, and @a z coordinates to this function to
/// ensure that these coordinates can be cast to a noise::int32 value.
///
/// Although you could do a straight cast from double to noise::int32, the
/// resulting value may differ between platforms.  By using this function,
/// you ensure that the resulting value is identical between platforms.
double makeInt32Range( const double n );

/// Performs cubic interpolation between two values bound between two other values.
///
/// @param n0 The value before the first value.
/// @param n1 The first value.
/// @param n2 The second value.
/// @param n3 The value after the second value.
/// @param a The alpha value.
///
/// @returns The interpolated value.
///
/// The alpha value should range from 0.0 to 1.0.  If the alpha value is 0.0, this
/// function returns @a n1.  If the alpha value is 1.0, this function returns @a n2.
double cubicInterp( const double n0, const double n1, const double n2, const double n3, const double a );

/// Performs linear interpolation between two values.
///
/// @param n0 The first value.
/// @param n1 The second value.
/// @param a The alpha value.
///
/// @returns The interpolated value.
///
/// The alpha value should range from 0.0 to 1.0.  If the alpha value is 0.0, this
/// function returns @a n0.  If the alpha value is 1.0, this function returns @a n1.
double noiselerp( const double n0, const double n1, const double a );

/// Maps a value onto a cubic S-curve.
///
/// @param a The value to map onto a cubic S-curve.
///
/// @returns The mapped value.
///
/// @a a should range from 0.0 to 1.0.
///
/// The derivitive of a cubic S-curve is zero at @a a = 0.0 and @a a =
/// 1.0
double sCurve3( const double a );

/// Maps a value onto a quintic S-curve.
///
/// @param a The value to map onto a quintic S-curve.
///
/// @returns The mapped value.
///
/// @a a should range from 0.0 to 1.0.
///
/// The first derivitive of a quintic S-curve is zero at @a a = 0.0 and
/// @a a = 1.0
///
/// The second derivitive of a quintic S-curve is zero at @a a = 0.0 and
/// @a a = 1.0
double sCurve5( const double a );

/// Converts latitude/longitude coordinates on a unit sphere into 3D Cartesian coordinates.
///
/// @param lat The latitude, in degrees.
/// @param lon The longitude, in degrees.
/// @param x On exit, this parameter contains the @a x coordinate.
/// @param y On exit, this parameter contains the @a y coordinate.
/// @param z On exit, this parameter contains the @a z coordinate.
///
/// @pre lat must range from @b -90 to @b +90.
/// @pre lon must range from @b -180 to @b +180.
void latLonToXYZ( const double lat, const double lon, double &x, double &y, double &z );

}
