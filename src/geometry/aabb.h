
#pragma once

#include "omath/vec3.h"
#include <iostream>

namespace orf_n {

template <typename T>
struct aabb_t {
	// member vars
	omath::vec3_t<T> m_min;

	omath::vec3_t<T> m_max;

	aabb_t<T>() {};

	aabb_t<T>( const omath::vec3_t<T>& min, const omath::vec3_t<T>& max ) :
			m_min{ min }, m_max{ max } {}

	virtual ~aabb_t() {};

	/**
	 * Returns the center of the bounding box
	 */
	const omath::vec3_t<T> get_center() const {
		return ( m_min + m_max ) * T{0.5};
	}

	const omath::vec3_t<T> get_size() const  {
		return m_max - m_min;
	}

	T get_diagonal_size() const {
		return omath::magnitude( m_max - m_min );
	}

	bool intersect_other( const aabb_t<T>& other ) const {
		return !( (other.m_max.x < m_min.x) || (other.m_min.x > m_max.x) ||
				  (other.m_max.y < m_min.y) || (other.m_min.y > m_max.y) ||
				  (other.m_max.z < m_min.z) || (other.m_min.z > m_max.z) );
	}

	T min_distance_from_point_sq( const omath::vec3_t<T>& point ) const {
		T dist{ 0.0 };
		if( point.x < m_min.x ) {
			T d{ point.x - m_min.x };
			dist += d * d;
		} else if( point.x > m_max.x ) {
			T d{ point.x - m_max.x };
			dist += d * d;
		}
		if( point.y < m_min.y ) {
			T d{ point.y - m_min.y };
			dist += d * d;
		} else if( point.y > m_max.y ) {
			T d{ point.y - m_max.y };
			dist += d * d;
		}
		if( point.z < m_min.z ) {
			T d{ point.z - m_min.z };
			dist += d * d;
		} else if( point.z > m_max.z ) {
			T d{ point.z - m_max.z };
			dist += d * d;
		}
		return dist;
	}

	T max_distance_from_point_sq( const omath::vec3_t<T>& point ) const {
		T k{ std::max( std::abs( point.x - m_min.x ), std::abs( point.x - m_max.x ) ) };
		T dist{ k * k };
		k = std::max( std::abs( point.y - m_min.y ), std::abs( point.y - m_max.y ) );
		dist += k * k;
		k = std::max( std::abs( point.z - m_min.z ), std::abs( point.z - m_max.z ) );
		dist += k * k;
		return dist;
	}

	bool intersect_sphere_sq( const omath::vec3_t<T>& center, double radius_sq ) const {
		return min_distance_from_point_sq( center ) <= radius_sq;
	}

	bool is_inside_sphere_sq( const omath::vec3_t<T>& center, double radius_sq ) const {
		return max_distance_from_point_sq( center ) <= radius_sq;
	}

	// @todo needs overwork to work with vectors
	bool intersect_ray( const omath::vec3_t<T>& ray_origin, const omath::vec3_t<T>& ray_direction, T& distance ) const {
		T tmin{ std::numeric_limits<T>::min() };
		T tmax{ std::numeric_limits<T>::max() };
		const T _rayOrigin[]{ ray_origin.x, ray_origin.y, ray_origin.z };
		const T _rayDirection[]{ ray_direction.x, ray_direction.y, ray_direction.z };
		const T _min[]{ m_min.x, m_min.y, m_min.z };
		const T _max[]{ m_max.x, m_max.y, m_max.z };
		const T EPSILON{ 1e-5 };
		for( int i{0}; i < 3; ++i ) {
			if ( std::abs( _rayDirection[i]) < EPSILON ) {
				// Parallel to the plane
				if( _rayOrigin[i] < _min[i] || _rayOrigin[i] > _max[i] )
					return false;
			} else {
				const T ood{ 1.0f / _rayDirection[i] };
				T t1{ ( _min[i] - _rayOrigin[i]) * ood };
				T t2{ ( _max[i] - _rayOrigin[i]) * ood };
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

	// returns aabb that encloses this one and the other one
	aabb_t<T> enclose_other( const aabb_t<T>& other ) const{
		omath::vec3_t<T> bmin, bmax;
		bmin.x = std::min( m_min.x, other.m_min.x );
		bmin.y = std::min( m_min.y, other.m_min.y );
		bmin.z = std::min( m_min.z, other.m_min.z );
		bmax.x = std::max( m_max.x, other.m_max.x );
		bmax.y = std::max( m_max.y, other.m_max.y );
		bmax.z = std::max( m_max.z, other.m_max.z );
		return aabb_t<T>{ bmin, bmax };
	}

	bool operator==( const aabb_t<T>& other ) const {
		return omath::compare_float( m_min, other.m_min ) && omath::compare_float( m_max, other.m_max );
	}

	T get_bounding_sphere_radius() const {
		return omath::magnitude( get_size() ) * T{0.5};
	}

	omath::vec3_t<T> get_vertex_positive( const omath::vec3_t<T>& normal ) const {
		omath::vec3_t<T> positive{ m_min };
		if( normal.x >= 0.0f )
			positive.x = m_max.x;
		if( normal.y >= 0.0f )
			positive.y = m_max.y;
		if( normal.z >= 0.0f )
			positive.z = m_max.z;
		return positive;
	}

	omath::vec3_t<T> get_vertex_negative( const omath::vec3_t<T>& normal ) const {
		omath::vec3_t<T> negative{ m_max };
		if( normal.x >= 0.0f )
			negative.x = m_min.x;
		if( normal.y >= 0.0f )
			negative.y = m_min.y;
		if( normal.z >= 0.0f )
			negative.z = m_min.z;
		return negative;
	}

	aabb_t<T> expand( T percentage ) const {
		omath::vec3_t<T> offset{ get_size() * percentage };
		return aabb_t<T>( m_min - offset, m_max + offset );
	}

};

typedef aabb_t<float> aabb;
typedef aabb_t<double> daabb;

}

template<typename T>
std::ostream& operator<<( std::ostream& o, const orf_n::aabb_t<T>& b ) {
	o << '(' << b.m_min << '/' << b.m_max << ')';
	return o;
}
