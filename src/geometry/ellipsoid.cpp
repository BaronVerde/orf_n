
#include <geometry/ellipsoid.h>
#include <omath/rotateVec.h>
#include <stdexcept>

namespace orf_n {

// All static
const omath::dvec3 ellipsoid::WGS84_ELLIPSOID{ 6378137.0, 6378137.0, 6356752.314245 };
const omath::dvec3 ellipsoid::WGS84_ELLIPSOID_SMALL{ 6378.137, 6378.137, 6356.752314245 };
const omath::dvec3 ellipsoid::WGS84_ELLIPSOID_SCALED{ 1.0, 1.0, 6378137.0 / 6356752.314245 };
const omath::dvec3 ellipsoid::UNIT_SPHERE{ 1.0, 1.0, 1.0 };
const omath::dvec3 ellipsoid::ONE_TO_FIVE{ 1.0, 0.2, 0.2 };
const omath::dvec3 ellipsoid::ONE_TO_TWO{ 1.0, 0.5f, 0.5 };
const omath::dvec3 ellipsoid::ONE_TO_THREE{ 1.0, 0.33, 0.33 };

ellipsoid::ellipsoid( const omath::dvec3 &radii ) :
		m_radii{ radii },
		m_radii_squared { m_radii * m_radii },
		m_radii_to_the_fourth { m_radii_squared * m_radii_squared },
		m_one_over_radii_squared { 1.0 / m_radii_squared } {}

ellipsoid::ellipsoid( const double x, const double y, const double z ) :
		ellipsoid{ omath::dvec3{ x, y, z } } {}

ellipsoid::~ellipsoid() {}

/**
 * Surface normals from cartesian coordinates.
 */
// static
omath::vec3 ellipsoid::centric_surface_normal( const omath::dvec3 &pos ) {
	return omath::normalize( pos );
}

omath::vec3 ellipsoid::geodetic_surface_normal( const omath::dvec3 &pos ) const {
	return omath::normalize( pos * m_one_over_radii_squared );
}

// static
omath::vec3 ellipsoid::geodetic_surface_normal( const geodetic &geodetic ) {
	const float cosLatitude{ static_cast<float>( std::cos( geodetic.get_latitude() ) ) };
	return omath::vec3{ cosLatitude * static_cast<float>( std::cos( geodetic.get_longitude() ) ),
						cosLatitude * static_cast<float>( std::sin( geodetic.get_longitude() ) ),
						static_cast<float>( std::sin( geodetic.get_latitude() ) ) };
}

omath::dvec3 ellipsoid::to_cartesian( const geodetic &geodetic ) const {
	const omath::dvec3 n{ geodetic_surface_normal( geodetic ) };
	const omath::dvec3 k{ m_radii_squared * n };
	const double gamma{ std::sqrt( (k.x * n.x) + (k.y * n.y) + (k.z * n.z) ) };
	// cartesian surface position
	const omath::dvec3 rSurface{ k / gamma };
	return rSurface + ( geodetic.get_height() * n );
}

const omath::dvec3 &ellipsoid::get_radii() const {
	return m_radii;
}

const omath::dvec3 &ellipsoid::get_radii_squared() const {
	return m_radii_squared;
}

const omath::dvec3 &ellipsoid::getOneOverRadiiSquared() const {
	return m_one_over_radii_squared;
}

void ellipsoid::set_position( const omath::dvec3 &position ) {
	m_position = position;
}

const omath::dvec3 &ellipsoid::get_position() const {
	return m_position;
}


double ellipsoid::get_minimum_radius() const {
	return std::min( m_radii.x, std::min( m_radii.y, m_radii.z ) );
}

double ellipsoid::get_maximum_radius() const {
	return std::max( m_radii.x, std::max( m_radii.y, m_radii.z ) );
}

std::vector<double> ellipsoid::intersections( const omath::dvec3 origin, const omath::dvec3 direction ) const {
	const omath::dvec3 dir{ omath::normalize( direction ) };
	double a = dir.x * dir.x * m_one_over_radii_squared.x +
			   dir.y * dir.y * m_one_over_radii_squared.y +
			   dir.z * dir.z * m_one_over_radii_squared.z;
	double b = 2.0 * ( origin.x * dir.x * m_one_over_radii_squared.x +
					   origin.y * dir.y * m_one_over_radii_squared.y +
					   origin.z * dir.z * m_one_over_radii_squared.z );
	double c = origin.x * origin.x * m_one_over_radii_squared.x +
			   origin.y * origin.y * m_one_over_radii_squared.y +
			   origin.z * origin.z * m_one_over_radii_squared.z - 1.0;
	// Solve the quadratic equation: ax^2 + bx + c = 0
	std::vector<double> result( 1 );
	const double discriminant{ b * b - 4.0 * a * c };
	if( discriminant < 0.0 ) {
		// no intersections
		result[0] = 0.0;
		return result;
	} else if( omath::compare_float( discriminant, 0.0 ) ) {
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
geodetic ellipsoid::to_geodetic( const omath::dvec3 &cartesianPosition ) const {
	const omath::dvec3 p{ scale_to_geodetic_surface( cartesianPosition ) };
	const omath::dvec3 h{ cartesianPosition - p };
	const double sign{ omath::dot( h, cartesianPosition ) };
	double height{0.0};
	if( sign < 0.0 )
		height = -omath::magnitude( h );
	else if( height > 0.0 )
		height = omath::magnitude( h );
	const omath::dvec3 n{ geodetic_surface_normal( p ) };
	return geodetic{ std::asin( n.z / omath::magnitude( n ) ),
					 std::atan2( n.y, n.x ),
					 height };
}

omath::dvec3 ellipsoid::scale_to_geodetic_surface( const omath::dvec3 &position ) const {
	const double beta{
		1.0 / std::sqrt( ( position.x * position.x ) * m_one_over_radii_squared.x +
						 ( position.y * position.y ) * m_one_over_radii_squared.y +
						 ( position.z * position.z ) * m_one_over_radii_squared.z )
	};
	const double n{ omath::magnitude( omath::dvec3{ beta * position * m_one_over_radii_squared } ) };
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
		da = 1.0 + ( alpha * m_one_over_radii_squared.x );
		db = 1.0 + ( alpha * m_one_over_radii_squared.y );
		dc = 1.0 + ( alpha * m_one_over_radii_squared.z );
		double da2 = da * da;
		double db2 = db * db;
		double dc2 = dc * dc;
		double da3 = da * da2;
		double db3 = db * db2;
		double dc3 = dc * dc2;
		s = x2 / ( m_radii_squared.x * da2 ) +
			y2 / ( m_radii_squared.y * db2 ) +
			z2 / ( m_radii_squared.z * dc2 ) - 1.0;
		dSdA = -2.0 * ( x2 / ( m_radii_to_the_fourth.x * da3 ) +
						y2 / ( m_radii_to_the_fourth.y * db3 ) +
						z2 / ( m_radii_to_the_fourth.z * dc3 ) );
	} while( std::abs(s) > 1.0e-10 );

	return omath::dvec3{ position.x / da, position.y / db, position.z / dc };
}	// scaleToGeodeticSurface()

omath::dvec3 ellipsoid::scale_to_geocentric_surface( const omath::dvec3 &position ) const {
	const double beta{ 1.0 / std::sqrt( ( position.x * position.x ) * m_one_over_radii_squared.x +
							  			( position.y * position.y ) * m_one_over_radii_squared.y +
										( position.z * position.z ) * m_one_over_radii_squared.z ) };
	return beta * position;
}	// scaleToGeocentricSurface()

std::vector<omath::dvec3> ellipsoid::compute_curve( const omath::dvec3 start,
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
		positions[i] = scale_to_geocentric_surface( omath::rotate( start, phi, normal ) );
	}
	positions[n+1] = stop;
	return positions;
}	// computeCurce();

// static
omath::vec2 ellipsoid::compute_texture_coordinate( const omath::vec3 &normal ) {
	return omath::vec2{ static_cast<float>( std::atan2( normal.y, normal.x ) * omath::ONE_OVER_TWO_PI + 0.5 ),
						static_cast<float>( std::asin( normal.z ) * omath::ONE_OVER_PI + 0.5 ) };
}

}
