
#include <geometry/Ellipsoid.h>
#include <omath/rotateVec.h>
#include <stdexcept>

namespace orf_n {

// All static
const omath::dvec3 Ellipsoid::WGS84_ELLIPSOID{ 6378137.0, 6378137.0, 6356752.314245 };
const omath::dvec3 Ellipsoid::WGS84_ELLIPSOID_SMALL{ 6378.137, 6378.137, 6356.752314245 };
const omath::dvec3 Ellipsoid::WGS84_ELLIPSOID_SCALED{ 1.0, 1.0, 6378137.0 / 6356752.314245 };
const omath::dvec3 Ellipsoid::UNIT_SPHERE{ 1.0, 1.0, 1.0 };
const omath::dvec3 Ellipsoid::ONE_TO_FIVE{ 1.0, 0.2, 0.2 };
const omath::dvec3 Ellipsoid::ONE_TO_TWO{ 1.0, 0.5f, 0.5 };
const omath::dvec3 Ellipsoid::ONE_TO_THREE{ 1.0, 0.33, 0.33 };

Ellipsoid::Ellipsoid( const omath::dvec3 &radii ) :
		m_radii{ radii },
		m_radiiSquared { m_radii * m_radii },
		m_radiiToTheFourth { m_radiiSquared * m_radiiSquared },
		m_oneOverRadiiSquared { 1.0 / m_radiiSquared } {}

Ellipsoid::Ellipsoid( const double x, const double y, const double z ) :
		Ellipsoid{ omath::dvec3{ x, y, z } } {}

Ellipsoid::~Ellipsoid() {}

/**
 * Surface normals from cartesian coordinates.
 */
// static
omath::vec3 Ellipsoid::centricSurfaceNormal( const omath::dvec3 &pos ) {
	return omath::normalize( pos );
}

omath::vec3 Ellipsoid::geodeticSurfaceNormal( const omath::dvec3 &pos ) const {
	return omath::normalize( pos * m_oneOverRadiiSquared );
}

// static
omath::vec3 Ellipsoid::geodeticSurfaceNormal( const Geodetic &geodetic ) {
	const float cosLatitude{ static_cast<float>( std::cos( geodetic.getLatitude() ) ) };
	return omath::vec3{ cosLatitude * static_cast<float>( std::cos( geodetic.getLongitude() ) ),
						cosLatitude * static_cast<float>( std::sin( geodetic.getLongitude() ) ),
						static_cast<float>( std::sin( geodetic.getLatitude() ) ) };
}

omath::dvec3 Ellipsoid::toCartesian( const Geodetic &geodetic ) const {
	const omath::dvec3 n{ geodeticSurfaceNormal( geodetic ) };
	const omath::dvec3 k{ m_radiiSquared * n };
	const double gamma{ std::sqrt( (k.x * n.x) + (k.y * n.y) + (k.z * n.z) ) };
	// cartesian surface position
	const omath::dvec3 rSurface{ k / gamma };
	return rSurface + ( geodetic.getHeight() * n );
}

const omath::dvec3 &Ellipsoid::getRadii() const {
	return m_radii;
}

const omath::dvec3 &Ellipsoid::getRadiiSquared() const {
	return m_radiiSquared;
}

const omath::dvec3 &Ellipsoid::getOneOverRadiiSquared() const {
	return m_oneOverRadiiSquared;
}

void Ellipsoid::setPosition( const omath::dvec3 &position ) {
	m_position = position;
}

const omath::dvec3 &Ellipsoid::getPosition() const {
	return m_position;
}


double Ellipsoid::getMinimumRadius() const {
	return std::min( m_radii.x, std::min( m_radii.y, m_radii.z ) );
}

double Ellipsoid::getMaximumRadius() const {
	return std::max( m_radii.x, std::max( m_radii.y, m_radii.z ) );
}

std::vector<double> Ellipsoid::intersections( const omath::dvec3 origin, const omath::dvec3 direction ) const {
	const omath::dvec3 dir{ omath::normalize( direction ) };
	double a = dir.x * dir.x * m_oneOverRadiiSquared.x +
			   dir.y * dir.y * m_oneOverRadiiSquared.y +
			   dir.z * dir.z * m_oneOverRadiiSquared.z;
	double b = 2.0 * ( origin.x * dir.x * m_oneOverRadiiSquared.x +
					   origin.y * dir.y * m_oneOverRadiiSquared.y +
					   origin.z * dir.z * m_oneOverRadiiSquared.z );
	double c = origin.x * origin.x * m_oneOverRadiiSquared.x +
			   origin.y * origin.y * m_oneOverRadiiSquared.y +
			   origin.z * origin.z * m_oneOverRadiiSquared.z - 1.0;
	// Solve the quadratic equation: ax^2 + bx + c = 0
	std::vector<double> result( 1 );
	const double discriminant{ b * b - 4.0 * a * c };
	if( discriminant < 0.0 ) {
		// no intersections
		result[0] = 0.0;
		return result;
	} else if( omath::compareFloat( discriminant, 0.0 ) ) {
		// one intersection at a tangent point
		result[0] = -0.5 * b / a;
		return result;
	}

	double t = -0.5 * (b + (b > 0.0 ? 1.0 : -1.0) * std::sqrt( discriminant ) );
	double root1 = t / a;
	double root2 = c / t;
	// Two intersections - return the smallest first.
	if( root1 < root2 ) {
		result.push_back( root1 );
		result.push_back( root2 );
	} else {
		result.push_back( root2 );
		result.push_back( root1 );
	}
	return result;
}	// intersections()

// @todo: check if this is correct: latittude and longitude !
Geodetic Ellipsoid::toGeodetic( const omath::dvec3 &cartesianPosition ) const {
	const omath::dvec3 p{ scaleToGeodeticSurface( cartesianPosition ) };
	const omath::dvec3 h{ cartesianPosition - p };
	const double sign{ omath::dot( h, cartesianPosition ) };
	double height{0.0};
	if( sign < 0.0 )
		height = -omath::magnitude( h );
	else if( height > 0.0 )
		height = omath::magnitude( h );
	const omath::dvec3 n{ geodeticSurfaceNormal( p ) };
	return Geodetic{ std::asin( n.z / omath::magnitude( n ) ),
					 std::atan2( n.y, n.x ),
					 height };
}

omath::dvec3 Ellipsoid::scaleToGeodeticSurface( const omath::dvec3 &position ) const {
	const double beta{
		1.0 / std::sqrt( ( position.x * position.x ) * m_oneOverRadiiSquared.x +
						 ( position.y * position.y ) * m_oneOverRadiiSquared.y +
						 ( position.z * position.z ) * m_oneOverRadiiSquared.z )
	};
	const double n{ omath::magnitude( omath::dvec3{ beta * position * m_oneOverRadiiSquared } ) };
	double alpha{ ( 1.0 - beta ) * ( omath::magnitude( position ) / n ) };
	const double x2{ position.x * position.x };
	const double y2{ position.y * position.y };
	const double z2{ position.z * position.z };
	double da{ 0.0 };
	double db{ 0.0 };
	double dc{ 0.0 };
	double s{ 0.0 };
	double dSdA{ 1.0 };
	do {
		alpha -= ( s / dSdA );
		da = 1.0 + ( alpha * m_oneOverRadiiSquared.x );
		db = 1.0 + ( alpha * m_oneOverRadiiSquared.y );
		dc = 1.0 + ( alpha * m_oneOverRadiiSquared.z );
		double da2 = da * da;
		double db2 = db * db;
		double dc2 = dc * dc;
		double da3 = da * da2;
		double db3 = db * db2;
		double dc3 = dc * dc2;
		s = x2 / ( m_radiiSquared.x * da2 ) +
			y2 / ( m_radiiSquared.y * db2 ) +
			z2 / ( m_radiiSquared.z * dc2 ) - 1.0;
		dSdA = -2.0 * ( x2 / ( m_radiiToTheFourth.x * da3 ) +
						y2 / ( m_radiiToTheFourth.y * db3 ) +
						z2 / ( m_radiiToTheFourth.z * dc3 ) );
	} while( std::abs(s) > 1.0e-10 );

	return omath::dvec3{ position.x / da, position.y / db, position.z / dc };
}	// scaleToGeodeticSurface()

omath::dvec3 Ellipsoid::scaleToGeocentricSurface( const omath::dvec3 &position ) const {
	const double beta{ 1.0 / std::sqrt( ( position.x * position.x ) * m_oneOverRadiiSquared.x +
							  			( position.y * position.y ) * m_oneOverRadiiSquared.y +
										( position.z * position.z ) * m_oneOverRadiiSquared.z ) };
	return beta * position;
}	// scaleToGeocentricSurface()

std::vector<omath::dvec3> Ellipsoid::computeCurve( const omath::dvec3 start,
												   const omath::dvec3 stop,
												   const double granularity ) const {
	if( granularity <= 0.0 )
		throw std::runtime_error( "Parameter error computing an ellipsoid curve." );
	const omath::dvec3 normal{ omath::normalize( omath::cross( start, stop ) ) };
	// angle bewteen start and stop
	const double theta{ angle( start, stop ) };
	const int n{ std::max( static_cast<int>( ( theta / granularity ) ) - 1, 0 ) };
	std::vector<omath::dvec3> positions(n+1);
	positions[0] = start;
	for( int i{1}; i <= n; ++i ) {
		const double phi{ i * granularity };
		positions[i] = scaleToGeocentricSurface( omath::rotate( start, phi, normal ) );
	}
	positions[n+1] = stop;
	return positions;
}	// computeCurce();

// static
omath::vec2 Ellipsoid::computeTextureCoordinate( const omath::vec3 &normal ) {
	return omath::vec2{ static_cast<float>( std::atan2( normal.y, normal.x ) * omath::ONE_OVER_TWO_PI + 0.5 ),
						static_cast<float>( std::asin( normal.z ) * omath::ONE_OVER_PI + 0.5 ) };
}

}
