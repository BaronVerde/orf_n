
#pragma once

#include <omath/vec3.h>
#include <vector>
//#include <limits>

namespace orf_n {

class OBB;
class Sphere;

typedef struct {
	bool colliding{ false };
	omath::dvec3 normal{ 0.0, 0.0, 1.0 };
	double depth = std::numeric_limits<double>::max();
	std::vector<omath::dvec3> contacts;

	void reset();

} collisionManifold_t;

collisionManifold_t findCollisionFeatures( const Sphere &a, const Sphere &b );

collisionManifold_t findCollisionFeatures( const OBB &a, const Sphere &b );

collisionManifold_t findCollisionFeatures( const OBB &a, const OBB &b );

}
