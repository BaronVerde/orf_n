
#include <noise/Misc.h>
#include <omath/common.h>

namespace noise {

double makeInt32Range( const double n ) {
	if( n >= 1073741824.0 )
		return( 2.0 * std::fmod( n, 1073741824.0 ) ) - 1073741824.0;
	else if( n <= -1073741824.0 )
		return( 2.0 * std::fmod( n, 1073741824.0 ) ) + 1073741824.0;
	else
		return n;
}

double cubicInterp( const double n0, const double n1, const double n2, const double n3, const double a ) {
	const double p{ ( n3 - n2 ) - ( n0 - n1 ) };
	const double q{ ( n0 - n1 ) - p };
	const double r{ n2 - n0 };
	const double s{ n1 };
	return p * a * a * a + q * a * a + r * a + s;
}

double noiselerp( const double n0, const double n1, const double a ) {
	return( ( 1.0 - a ) * n0 ) + ( a * n1 );
}

double sCurve3( const double a ) {
	return( a * a * ( 3.0 - 2.0 * a ) );
}

double sCurve5( const double a ) {
	const double a3{ a * a * a };
    const double a4{ a3 * a };
    const double a5{ a4 * a };
    return( 6.0 * a5 ) - ( 15.0 * a4 ) + ( 10.0 * a3 );
}

void latLonToXYZ( const double lat, const double lon, double &x, double &y, double &z ) {
	const double rLat{ omath::radians( lat ) };
	const double rLon{ omath::radians( lon ) };
	const double r{ std::cos( rLat  ) };
	x = r * std::cos( rLon );
	y = std::sin( rLat );
	z = r * std::sin( rLon );
}

}
