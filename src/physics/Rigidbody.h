
/**
 * Rigidbody base class, for bodies with mass but without volume.
 */

#pragma once

#include <vector>

namespace orf_n {

class OBB;
class DrawPrimitives;

class Rigidbody {
public:
	typedef enum : unsigned int {
		BASE = 0, PARTICLE, SPHERE, BOX
	} rigidbody_t;

	Rigidbody() : m_type{BASE} {};

	virtual ~Rigidbody() {};

	/**
	 * May be called for actual drawing from the renderer.
	 * @todo: only draws primitives for now. Later, drawing must be done in the renderer.
	 */
	virtual void render( const DrawPrimitives *const drawPrimitives ) {};

	/**
	 * Sum up and accumulate all the forces that act on the body.
	 */
	virtual void applyForces() {};

	/**
	 * Integrate the new position of the body. Called in a fixed timestep !
	 */
	virtual void update( const float deltaTime ) {};

	/**
	 * Solve constraints. Keep bodies from moving through each other and obstacles.
	 */
	virtual void solveConstraints( const std::vector<OBB> &constraints ) {};

	bool hasVolume() {
		return m_type == SPHERE || m_type == BOX;
	}

protected:
	rigidbody_t m_type{ BASE };

};

}
