
#include <geometry/AABB.h>
#include <algorithm>
#include <limits>
#include <ostream>

namespace orf_n {

AABB::AABB() {}

AABB::AABB( const omath::dvec3 &min, const omath::dvec3 &max )  :
		m_min{ min }, m_max{ max } {}

AABB::~AABB() {}

const omath::dvec3 AABB::getCenter() const {
	return ( m_min + m_max ) * 0.5;
}

const omath::dvec3 AABB::getSize() const {
	return m_max - m_min;
}

double AABB::getDiagonalSize() const  {
	return omath::magnitude( m_max - m_min );
}

bool AABB::intersectOther( const AABB &other ) const {
	return !( (other.m_max.x < m_min.x) || (other.m_min.x > m_max.x) ||
			  (other.m_max.y < m_min.y) || (other.m_min.y > m_max.y) ||
			  (other.m_max.z < m_min.z) || (other.m_min.z > m_max.z) );
}

double AABB::minDistanceFromPointSq( const omath::dvec3 &point ) const {
	double dist{ 0.0 };
	if( point.x < m_min.x ) {
		double d{ point.x - m_min.x };
		dist += d * d;
	} else if( point.x > m_max.x ) {
		double d{ point.x - m_max.x };
		dist += d * d;
	}
	if( point.y < m_min.y ) {
		double d{ point.y - m_min.y };
		dist += d * d;
	} else if( point.y > m_max.y ) {
		double d{ point.y - m_max.y };
		dist += d * d;
	}
	if( point.z < m_min.z ) {
		double d{ point.z - m_min.z };
		dist += d * d;
	} else if( point.z > m_max.z ) {
		double d{ point.z - m_max.z };
		dist += d * d;
	}
	return dist;
}

double AABB::maxDistanceFromPointSq( const omath::dvec3 &point ) const {
	double k{ std::max( std::abs( point.x - m_min.x ), std::abs( point.x - m_max.x ) ) };
	double dist{ k * k };
	k = std::max( std::abs( point.y - m_min.y ), std::abs( point.y - m_max.y ) );
	dist += k * k;
	k = std::max( std::abs( point.z - m_min.z ), std::abs( point.z - m_max.z ) );
	dist += k * k;
	return dist;
}

bool AABB::intersectSphereSq( const omath::dvec3 &center, double radiusSq ) const {
	return minDistanceFromPointSq( center ) <= radiusSq;
}

bool AABB::isInsideSphereSq( const omath::dvec3 & center, double radiusSq ) const {
	return maxDistanceFromPointSq( center ) <= radiusSq;
}

// @todo needs overwork to work with vectors
bool AABB::intersectRay( const omath::dvec3 &rayOrigin, const omath::dvec3 &rayDirection, double &distance ) const {
	double tmin{ std::numeric_limits<double>::min() };
	double tmax{ std::numeric_limits<double>::max() };
	const double _rayOrigin[]{ rayOrigin.x, rayOrigin.y, rayOrigin.z };
	const double _rayDirection[]{ rayDirection.x, rayDirection.y, rayDirection.z };
	const double _min[]{ m_min.x, m_min.y, m_min.z };
	const double _max[]{ m_max.x, m_max.y, m_max.z };
	const double EPSILON{ 1e-5 };
	for( int i{0}; i < 3; ++i ) {
		if ( std::abs( _rayDirection[i]) < EPSILON ) {
			// Parallel to the plane
			if( _rayOrigin[i] < _min[i] || _rayOrigin[i] > _max[i] )
				return false;
		} else {
			const double ood{ 1.0f / _rayDirection[i] };
			double t1{ ( _min[i] - _rayOrigin[i]) * ood };
			double t2{ ( _max[i] - _rayOrigin[i]) * ood };
			if (t1 > t2)
				std::swap( t1, t2 );
			if (t1 > tmin)
				tmin = t1;
			if (t2 < tmax)
				tmax = t2;
			if (tmin > tmax)
				return false;
		}
	}
	distance = tmin;
	return true;
}

AABB AABB::encloseOther( const AABB &other ) const {
	omath::dvec3 bmin, bmax;
	bmin.x = std::min( m_min.x, other.m_min.x );
	bmin.y = std::min( m_min.y, other.m_min.y );
	bmin.z = std::min( m_min.z, other.m_min.z );
	bmax.x = std::max( m_max.x, other.m_max.x );
	bmax.y = std::max( m_max.y, other.m_max.y );
	bmax.z = std::max( m_max.z, other.m_max.z );
	return AABB{ bmin, bmax };
}

bool AABB::operator==( const AABB &other ) const {
	return omath::compareFloat( m_min, other.m_min ) && omath::compareFloat( m_max, other.m_max );
}

double AABB::getBoundingSphereRadius() const {
	return omath::magnitude( getSize() ) * 0.5;
}

omath::dvec3 AABB::getVertexPositive( const omath::vec3 &normal ) const {
	omath::dvec3 positive{ m_min };
	if( normal.x >= 0.0f )
		positive.x = m_max.x;
	if( normal.y >= 0.0f )
		positive.y = m_max.y;
	if( normal.z >= 0.0f )
		positive.z = m_max.z;
	return positive;
}

omath::dvec3 AABB::getVertexNegative( const omath::vec3 &normal ) const {
	omath::dvec3 negative{ m_max };
	if( normal.x >= 0.0f )
		negative.x = m_min.x;
	if( normal.y >= 0.0f )
		negative.y = m_min.y;
	if( normal.z >= 0.0f )
		negative.z = m_min.z;
	return negative;
}

AABB AABB::expand( double percentage ) const {
	omath::dvec3 offset{ getSize() * percentage };
	return AABB( m_min - offset, m_max + offset );
}

}

std::ostream &operator<<( std::ostream &o, const orf_n::AABB &b ) {
	o << '(' << b.m_min << ")/(" << b.m_max << ')';
	return o;
}
