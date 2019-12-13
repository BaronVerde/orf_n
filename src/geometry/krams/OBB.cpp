
#include <geometry/krams/OBB.h>
#include <geometry/Line.h>
#include <geometry/Plane.h>
#include <omath/mat3.h>

namespace orf_n {

OBB::OBB() : m_halfSize{ 1.0f, 1.0f, 1.0f } {}

OBB::OBB( const omath::vec3 &position, const omath::vec3 &halfSize ) :
				m_position{ position }, m_halfSize{ halfSize } {}

OBB::OBB( const omath::vec3 &position, const omath::vec3 &halfSize, const omath::mat3 &orientation ) :
				m_position{ position }, m_halfSize{ halfSize }, m_orientation{ orientation } {}

void OBB::setPosition( const omath::vec3 &position ) {
	m_position = position;
}

bool OBB::containsPoint( const omath::vec3 &point ) const {
	const omath::vec3 dir{ point - m_position };
	// Project the point on each of the axes of the obb
	for( int i{0}; i < 3; ++i ) {
		// @todo: is this correct ? Are the axes columns vectors ? Or rather transposed values ?
		float distance{ omath::dot( dir, m_orientation[i] ) };
		if( distance > m_halfSize[i] )
			return false;
		if( distance < -m_halfSize[i] )
			return false;
	}
	return true;
}

// Returns closest point on the obb relative to given point
omath::vec3 OBB::closestPoint( const omath::vec3 &point ) const {
	omath::vec3 result{ m_position };
	const omath::vec3 dir{ point - m_position };
	for( int i{0}; i < 3; ++i) {
		// @todo: is this correct ? Are the axes columns vectors ? Or rather transposed values ?
		float distance{ omath::dot( dir, m_orientation[i] ) };
		if( distance > m_halfSize[i] )
			distance = m_halfSize[i];
		if( distance < -m_halfSize[i] )
			distance = -m_halfSize[i];
		result += m_orientation[i] * distance;
	}
	return result;
}

// Return projected interval on axis. .x = min, .y = max
omath::vec2 OBB::getInterval( const omath::vec3 &axis ) const {
	const std::array<omath::vec3, 8> vertices{ getVertices() };
	omath::vec2 result;
	result.x = result.y = omath::dot( axis, vertices[0] );
	for ( int i{1}; i < 8; ++i ) {
		const float projection{ omath::dot( axis, vertices[i] ) };
		result.x = projection < result.x ? projection : result.x;
		result.y = projection > result.y ? projection : result.y;
	}
	return result;
}

// bool OverlapOnAxis(const AABB& aabb, const OBB& obb, const vec3& axis);

bool OBB::overlapOnAxis( const OBB &other, const omath::vec3 &axis ) const {
	omath::vec2 a{ getInterval( axis ) };
	omath::vec2 b{ other.getInterval( axis ) };
	return ( b.x <= a.y ) && ( a.x <= b.y );
}

// bool OverlapOnAxis(const OBB& obb, const Triangle& triangle, const vec3& axis);

//bool SphereOBB(const Sphere& sphere, const OBB& obb);

//bool AABBOBB(const AABB& aabb, const OBB& obb);

//bool AABBPlane(const AABB& aabb, const Plane& plane);

bool OBB::intersectsOther( const OBB &other ) const {
	// Axes of the boxes
	omath::vec3 test[15] {
		m_orientation[0], m_orientation[1], m_orientation[2],
		other.m_orientation[0], other.m_orientation[1], other.m_orientation[2],
	};
	// Fill out rest of axes needed for SAT test
	for( int i{0}; i < 3; ++i ) {
		test[6 + i * 3 + 0] = omath::cross( test[i], test[0] );
		test[6 + i * 3 + 1] = omath::cross( test[i], test[1] );
		test[6 + i * 3 + 2] = omath::cross( test[i], test[2] );
	}
	for( int i{0}; i < 15; ++i ) {
		if( !overlapOnAxis( other, test[i] ) )
			return false;
	}
	// Seperating axis not found
	return true;
}

const omath::vec3 &OBB::getPosition() const {
	return m_position;
}

const omath::mat3 &OBB::getOrientation() const {
	return m_orientation;
}

const omath::vec3 &OBB::getHalfSize() const {
	return m_halfSize;
}

//bool OBBPlane(const OBB& obb, const Plane& plane);

//bool TriangleOBB(const Triangle& t, const OBB& o);

// Find collision features for boxes, with helper functions
std::array<omath::vec3, 8> OBB::getVertices() const {
	// @todo: is this correct ? If not, orientation must be transposed
	std::array<omath::vec3, 8> v{
		m_position + m_orientation[0] * m_halfSize[0] + m_orientation[1] * m_halfSize[1] + m_orientation[2] * m_halfSize[2],
		m_position - m_orientation[0] * m_halfSize[0] + m_orientation[1] * m_halfSize[1] + m_orientation[2] * m_halfSize[2],
		m_position + m_orientation[0] * m_halfSize[0] - m_orientation[1] * m_halfSize[1] + m_orientation[2] * m_halfSize[2],
		m_position + m_orientation[0] * m_halfSize[0] + m_orientation[1] * m_halfSize[1] - m_orientation[2] * m_halfSize[2],
		m_position - m_orientation[0] * m_halfSize[0] - m_orientation[1] * m_halfSize[1] - m_orientation[2] * m_halfSize[2],
		m_position + m_orientation[0] * m_halfSize[0] - m_orientation[1] * m_halfSize[1] - m_orientation[2] * m_halfSize[2],
		m_position - m_orientation[0] * m_halfSize[0] + m_orientation[1] * m_halfSize[1] - m_orientation[2] * m_halfSize[2],
		m_position - m_orientation[0] * m_halfSize[0] - m_orientation[1] * m_halfSize[1] + m_orientation[2] * m_halfSize[2]
	};
	return v;
}

std::array<Line, 12> OBB::getEdges() const {
	std::array<Line, 12> result;
	std::array<omath::vec3, 8> vertices{ getVertices() };
	// Indices of edge-vertices
	int index[12][2] {
		{6,1}, {6,3}, {6,4}, {2,7}, {2,5}, {2,0},
		{0,1}, {0,3}, {7,1}, {7,4}, {4,5}, {5,3}
	};
	for( int j{0}; j < 12; ++j )
		result[j] = Line( vertices[index[j][0]], vertices[index[j][1]] );
	return result;
}

std::array<Plane, 6> OBB::getPlanes() const {
	std::array<Plane, 6> result {
		Plane{ m_orientation[0], omath::dot( m_orientation[0], m_position + m_orientation[0] * m_halfSize.x ) },
		Plane{ -m_orientation[0], -omath::dot( m_orientation[0], m_position - m_orientation[0] * m_halfSize.x ) },
		Plane{ m_orientation[1], omath::dot( m_orientation[1], m_position + m_orientation[1] * m_halfSize.y ) },
		Plane{ -m_orientation[1], -omath::dot( m_orientation[1], m_position -m_orientation[1] * m_halfSize.y ) },
		Plane{ m_orientation[2], omath::dot( m_orientation[2], m_position + m_orientation[2] * m_halfSize.z ) },
		Plane{ -m_orientation[2], -omath::dot( m_orientation[2], m_position -m_orientation[2] * m_halfSize.z ) }
	};
	return result;
}

std::vector<omath::vec3> OBB::clipEdgesToOBB( const OBB &other ) const {
	const std::array<Line, 12> edges{ getEdges() };
	const std::array<Plane, 6> planes{ other.getPlanes() };
	std::vector<omath::vec3> result( 12 );
	omath::vec3 intersection;
	// For every other plane loop through this' edges
	for( int i{0}; i < 6; ++i ) {
		for( int j{0}; j < 12; ++j ) {
			// If edge and plane intersect return intersection point
			if( edges[j].clipToPlane( planes[i], intersection ) )
				if( containsPoint( intersection ) )
					result.push_back( intersection );
		}
	}
	return result;
}

float OBB::penetrationDepth( const OBB &other, const omath::vec3 &axis, bool *shouldNormalFlip ) const {
	// Get both intervals
	const omath::vec2 i1{ getInterval( omath::normalize( axis ) ) };
	const omath::vec2 i2{ getInterval( omath::normalize( axis ) ) };
	// Early out if no overlap
	if( !( ( i2.x <= i1.y ) && ( i1.x <= i2.y ) ) )
		return 0.0f;
	// Find length, lowest and highest
	const float len1{ i1.y - i1.x };
	const float len2{ i2.y - i2.x };
	const float min{ std::min( i1.x, i2.x ) };
	const float max{ std::max( i1.y, i2.y ) };
	// Length of combined interval
	const float comboLength{ max - min };
	// If other box is in front of this one the collision normal should be flipped
	if( shouldNormalFlip != nullptr )
		*shouldNormalFlip = ( i2.x < i1.x );
	return ( len1 + len2 ) - comboLength;
}


}
