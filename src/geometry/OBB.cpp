
#include <geometry/Line.h>
#include <geometry/OBB.h>
#include <geometry/Plane.h>
#include <omath/mat3.h>

namespace orf_n {

OBB::OBB() {}

OBB::OBB( const omath::dvec3 &position, const omath::dvec3 &halfSize ) :
				m_position{ position }, m_halfSize{ halfSize } {}

OBB::OBB( const omath::dvec3 &position, const omath::dvec3 &halfSize, const omath::dmat3 &orientation ) :
				m_position{ position }, m_halfSize{ halfSize }, m_orientation{ orientation } {}

void OBB::setPosition( const omath::dvec3 &position ) {
	m_position = position;
}

bool OBB::containsPoint( const omath::dvec3 &point ) const {
	const omath::dvec3 dir{ point - m_position };
	// Project the point on each of the axes of the obb
	for( int i{0}; i < 3; ++i ) {
		// @todo: is this correct ? Are the axes columns vectors ? Or rather transposed values ?
		double distance{ omath::dot( dir, m_orientation[i] ) };
		if( distance > m_halfSize[i] )
			return false;
		if( distance < -m_halfSize[i] )
			return false;
	}
	return true;
}

// Returns closest point on the obb relative to given point
omath::dvec3 OBB::closestPoint( const omath::dvec3 &point ) const {
	omath::dvec3 result{ m_position };
	const omath::dvec3 dir{ point - m_position };
	for( int i{0}; i < 3; ++i) {
		// @todo: is this correct ? Are the axes columns vectors ? Or rather transposed values ?
		double distance{ omath::dot( dir, m_orientation[i] ) };
		if( distance > m_halfSize[i] )
			distance = m_halfSize[i];
		if( distance < -m_halfSize[i] )
			distance = -m_halfSize[i];
		result += m_orientation[i] * distance;
	}
	return result;
}

// Return projected interval on axis. .x = min, .y = max
omath::dvec2 OBB::getInterval( const omath::dvec3 &axis ) const {
	const std::array<omath::dvec3, 8> vertices{ getVertices() };
	omath::dvec2 result;
	result.x = result.y = omath::dot( axis, vertices[0] );
	for ( int i{1}; i < 8; ++i ) {
		const double projection{ omath::dot( axis, vertices[i] ) };
		result.x = projection < result.x ? projection : result.x;
		result.y = projection > result.y ? projection : result.y;
	}
	return result;
}

// bool OverlapOnAxis(const AABB& aabb, const OBB& obb, const dvec3& axis);

bool OBB::overlapOnAxis( const OBB &other, const omath::dvec3 &axis ) const {
	omath::vec2 a{ getInterval( axis ) };
	omath::vec2 b{ other.getInterval( axis ) };
	return ( b.x <= a.y ) && ( a.x <= b.y );
}

// bool OverlapOnAxis(const OBB& obb, const Triangle& triangle, const dvec3& axis);

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

const omath::dvec3 &OBB::getPosition() const {
	return m_position;
}

const omath::dmat3 &OBB::getOrientation() const {
	return m_orientation;
}

const omath::dvec3 &OBB::getHalfSize() const {
	return m_halfSize;
}

//bool OBBPlane(const OBB& obb, const Plane& plane);

//bool TriangleOBB(const Triangle& t, const OBB& o);

// Find collision features for boxes, with helper functions
std::array<omath::dvec3, 8> OBB::getVertices() const {
	// @todo: is this correct ? If not, orientation must be transposed
	std::array<omath::dvec3, 8> v{
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
	std::array<omath::dvec3, 8> vertices{ getVertices() };
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

std::vector<omath::dvec3> OBB::clipEdgesToOBB( const OBB &other ) const {
	const std::array<Line, 12> edges{ getEdges() };
	const std::array<Plane, 6> planes{ other.getPlanes() };
	std::vector<omath::dvec3> result( 12 );
	omath::dvec3 intersection;
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

double OBB::penetrationDepth( const OBB &other, const omath::dvec3 &axis, bool *shouldNormalFlip ) const {
	// Get both intervals
	const omath::vec2 i1{ getInterval( omath::normalize( axis ) ) };
	const omath::vec2 i2{ other.getInterval( omath::normalize( axis ) ) };
	// Early out if no overlap
	if( !( ( i2.x <= i1.y ) && ( i1.x <= i2.y ) ) )
		return 0.0;
	// Find length, lowest and highest
	const double len1{ i1.y - i1.x };
	const double len2{ i2.y - i2.x };
	const double min{ std::min( i1.x, i2.x ) };
	const double max{ std::max( i1.y, i2.y ) };
	// Length of combined interval
	const double comboLength{ max - min };
	// If other box is in front of this one the collision normal should be flipped
	if( shouldNormalFlip != nullptr )
		*shouldNormalFlip = ( i2.x < i1.x );
	return ( len1 + len2 ) - comboLength;
}


}
