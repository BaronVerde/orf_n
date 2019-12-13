
#pragma once

#include <omath/mat3.h>
#include <array>
#include <vector>

namespace orf_n {

class Line;
class Plane;

class OBB {
public:
	OBB();

	OBB( const omath::vec3 &position, const omath::vec3 &halfSize );

	OBB( const omath::vec3 &position, const omath::vec3 &halfSize, const omath::mat3 &orientation );

	bool containsPoint( const omath::vec3 &point ) const;

	// Returns closest point on the obb relative to given point
	omath::vec3 closestPoint( const omath::vec3 &point ) const;

	// Return projected interval on axis. .x = min, .y = max
	omath::vec2 getInterval( const omath::vec3 &axis ) const;

	// bool OverlapOnAxis(const AABB& aabb, const OBB& obb, const vec3& axis);

	bool overlapOnAxis( const OBB &other, const omath::vec3 &axis ) const;

	// bool OverlapOnAxis(const OBB& obb, const Triangle& triangle, const vec3& axis);

	//bool SphereOBB(const Sphere& sphere, const OBB& obb);

	//bool AABBOBB(const AABB& aabb, const OBB& obb);

	//bool AABBPlane(const AABB& aabb, const Plane& plane);

	bool intersectsOther( const OBB &other ) const;

	void setPosition( const omath::vec3 &position );

	const omath::vec3 &getPosition() const;

	const omath::mat3 &getOrientation() const;

	const omath::vec3 &getHalfSize() const;

	//bool OBBPlane(const OBB& obb, const Plane& plane);

	//bool TriangleOBB(const Triangle& t, const OBB& o);

	std::array<omath::vec3, 8> getVertices() const;

	std::array<Line, 12> getEdges() const;

	std::array<Plane, 6> getPlanes() const;

	/**
	 * Clip the edges of this obb to the planes ot the other obb.
	 * Variable point number between 0 and max. number of edges (12).
	 */
	std::vector<omath::vec3> clipEdgesToOBB( const OBB &other ) const;

	/**
	 * Returns the penetration depth on a provided axis for another box.
	 * Ptr to a bool, if provided, will tell if other is in front of this box
	 * and collision normal will have to be flipped.
	 */
	float penetrationDepth( const OBB &other, const omath::vec3 &axis, bool *shouldNormalFlip = nullptr ) const;

private:
	omath::vec3 m_position{ 0.0f, 0.0f, 0.0f };

	omath::vec3 m_halfSize{ 1.0f, 1.0f, 1.0f };

	// To store the rotation matrix
	omath::mat3 m_orientation{ 1.0f };

};

}
