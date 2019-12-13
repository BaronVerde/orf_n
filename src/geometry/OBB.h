
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

	OBB( const omath::dvec3 &position, const omath::dvec3 &halfSize );

	OBB( const omath::dvec3 &position, const omath::dvec3 &halfSize, const omath::dmat3 &orientation );

	bool containsPoint( const omath::dvec3 &point ) const;

	// Returns closest point on the obb relative to given point
	omath::dvec3 closestPoint( const omath::dvec3 &point ) const;

	// Return projected interval on axis. .x = min, .y = max
	omath::dvec2 getInterval( const omath::dvec3 &axis ) const;

	// bool OverlapOnAxis(const AABB& aabb, const OBB& obb, const dvec3& axis);

	bool overlapOnAxis( const OBB &other, const omath::dvec3 &axis ) const;

	// bool OverlapOnAxis(const OBB& obb, const Triangle& triangle, const dvec3& axis);

	//bool SphereOBB(const Sphere& sphere, const OBB& obb);

	//bool AABBOBB(const AABB& aabb, const OBB& obb);

	//bool AABBPlane(const AABB& aabb, const Plane& plane);

	bool intersectsOther( const OBB &other ) const;

	void setPosition( const omath::dvec3 &position );

	const omath::dvec3 &getPosition() const;

	const omath::dmat3 &getOrientation() const;

	const omath::dvec3 &getHalfSize() const;

	//bool OBBPlane(const OBB& obb, const Plane& plane);

	//bool TriangleOBB(const Triangle& t, const OBB& o);

	std::array<omath::dvec3, 8> getVertices() const;

	std::array<Line, 12> getEdges() const;

	std::array<Plane, 6> getPlanes() const;

	/**
	 * Clip the edges of this obb to the planes ot the other obb.
	 * Variable point number between 0 and max. number of edges (12).
	 */
	std::vector<omath::dvec3> clipEdgesToOBB( const OBB &other ) const;

	/**
	 * Returns the penetration depth on a provided axis for another box.
	 * Ptr to a bool, if provided, will tell if other is in front of this box
	 * and collision normal will have to be flipped.
	 */
	double penetrationDepth( const OBB &other, const omath::dvec3 &axis, bool *shouldNormalFlip = nullptr ) const;

private:
	omath::dvec3 m_position{ 0.0 };

	omath::dvec3 m_halfSize{ 1.0 };

	// To store the rotation matrix
	omath::dmat3 m_orientation{ 1.0 };

};

}
