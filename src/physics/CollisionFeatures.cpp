
#include <base/globals.h>
#include <geometry/Line.h>
#include <geometry/OBB.h>
#include <geometry/Plane.h>
#include <geometry/Sphere.h>
#include <physics/CollisionFeatures.h>

namespace orf_n {

void collisionManifold_t::reset() {
	colliding = false;
	normal = omath::dvec3{ 0.0, 0.0, 1.0 };
	depth = std::numeric_limits<double>::max();
	contacts.clear();
}

collisionManifold_t findCollisionFeatures( const Sphere &a, const Sphere &b ) {
	collisionManifold_t result;
	const double r{ a.getRadius() + b.getRadius() };
	const omath::dvec3 d{ b.getPosition() - a.getPosition() };
	// Distance to large, no intersection
	if( omath::magnitudeSq( d ) - r * r > 0 || omath::compareFloat( omath::magnitudeSq( d ), 0.0 ) )
		return result;
	const omath::dvec3 dn{ omath::normalize( d ) };
	result.colliding = true;
	result.normal = dn;
	result.depth = std::abs( magnitudeSq( dn ) - r ) * 0.5;
	// dtp - Distance to intersection point
	const double dtp{ a.getRadius() - result.depth };
	const omath::dvec3 contact{ a.getPosition() + d * dtp };
	result.contacts.push_back( contact );
	return result;
}

collisionManifold_t findCollisionFeatures( const OBB &a, const Sphere &b ) {
	collisionManifold_t result;
	const omath::dvec3 closestPoint{ a.closestPoint( b.getPosition() ) };
	const double distanceSquared{ omath::magnitudeSq( closestPoint - b.getPosition() ) };
	// Early out if no intersection
	if( distanceSquared > b.getRadius() * b.getRadius() )
		return result;
	// Intersection ...
	omath::dvec3 normal;
	if( omath::compareFloat( distanceSquared, 0.0 ) ) {
		const double mSq{ omath::magnitudeSq( closestPoint - a.getPosition() ) };
		if( omath::compareFloat( mSq, 0.0 ) )
			return result;
		// Closest point is at the center of the sphere
		normal = omath::normalize( closestPoint - a.getPosition() );
	} else
		normal = omath::normalize( b.getPosition() - closestPoint );
	// Fill out papers
	const omath::dvec3 outsidePoint{ b.getPosition() - normal * b.getRadius() };
	const double distance{ omath::magnitude( closestPoint - outsidePoint ) };
	result.colliding = true;
	result.contacts.push_back( closestPoint + ( outsidePoint - closestPoint ) * 0.5 );
	result.normal = normal;
	result.depth = distance * 0.5;
	return result;
}

collisionManifold_t findCollisionFeatures( const OBB &a, const OBB &b ) {
	collisionManifold_t result;
	// Store axes for SAT test
	omath::dvec3 test[15]{
		a.getOrientation()[0], a.getOrientation()[1], a.getOrientation()[2],
		b.getOrientation()[0], b.getOrientation()[1], b.getOrientation()[2]
	};
	for( int i{0}; i < 3; ++i ) {
		test[6 + i * 3 + 0] = omath::cross( test[i], test[0] );
		test[6 + i * 3 + 1] = omath::cross( test[i], test[1] );
		test[6 + i * 3 + 2] = omath::cross( test[i], test[2] );
	}
	// Temp var for the collision normal
	omath::dvec3 *hitNormal{ nullptr };
	bool shouldFlip;
	// Iterate through the axes
	for( int i{0}; i < 15; ++i ) {
		// Robust edge case checking
		if( test[i].x < 0.00001f )
			test[i].x = 0.0f;
		if( test[i].y < 0.00001f )
			test[i].y = 0.0f;
		if( test[i].z < 0.00001f )
			test[i].z = 0.0f;
		if( magnitudeSq( test[i]) < 0.001f )
			continue;
		const double depth{ a.penetrationDepth( b, test[i], &shouldFlip ) };
		// Early out if no intersection
		if( depth <= 0.0f )
			return result;
		else if( depth < result.depth) {
			if( shouldFlip )
				test[i] = -test[i];
			// Store depth and normal
			result.depth = depth;
			hitNormal = &test[i];
		}
	}
	// No normal found ? Then there is no collision
	if( nullptr == hitNormal )
		return result;
	const omath::dvec3 axis{ omath::normalize( *hitNormal ) };
	// Clip boxes against each other, so we have a list of intersection points.
	const std::vector<omath::dvec3> c1{ a.clipEdgesToOBB( b ) };
	const std::vector<omath::dvec3> c2{ b.clipEdgesToOBB( a ) };
	result.contacts.reserve( c1.size() + c2.size() );
	result.contacts.insert( result.contacts.end(), c1.begin(), c1.end() );
	result.contacts.insert( result.contacts.end(), c2.begin(), c2.end() );
	// Project results on a shared plane constructed from collision normals
	const omath::dvec2 i{ a.getInterval( axis ) };
	const double distance{ ( i.y - i.x ) * 0.5f - result.depth * 0.5 };
	const omath::dvec3 pointOnPlane{ a.getPosition() + axis * distance };
	// Store results of projection
	for( int i{ result.contacts.size() - 1 }; i >= 0; --i ) {
		const omath::dvec3 contact{ result.contacts[i] };
		result.contacts[i] = contact + ( axis * omath::dot( axis, pointOnPlane - contact ) );
		// Check for and avoid duplicates
		for( int j{ result.contacts.size() - 1 }; j > i; --j ) {
			if( magnitudeSq( result.contacts[j] - result.contacts[i] ) < 0.0001f ) {
				result.contacts.erase(result.contacts.begin() + j);
				break;
			}
		}
	}
	result.colliding = true;
	result.normal = axis;
	return result;
}

}
