
#pragma once

#include <geometry/OBB.h>
#include <omath/common.h>
#include <physics/CollisionFeatures.h>
#include <physics/Rigidbody.h>
#include <physics/RigidbodyVolume.h>
//#include "Renderer/DrawPrimitives.h"

namespace orf_n {

class PhysicsSystem {
public:
	PhysicsSystem() {
		m_colliders1.reserve( 100 );
		m_colliders2.reserve( 100 );
		m_results.reserve( 100 );
	}

	virtual ~PhysicsSystem() {
		// cleanup;
	}

	/**
	 * Must be called in a fixed interval !
	 */
	void update( const float deltaTime ) {
		m_colliders1.clear();
		m_colliders2.clear();
		m_results.clear();
		// @todo set colors in shaders and shader vars
		const size_t bs{ m_bodies.size() };
		// Loop through rigidbodies to find colliding couples
		for( size_t i{0}; i < bs; ++i ) {
			for( size_t j{i}; j < bs; ++j ) {
				if( i == j )
					continue;
				collisionManifold_t result;
				// Can the two bodies collide ?
				if( m_bodies[i]->hasVolume() && m_bodies[j]->hasVolume() ) {
					RigidbodyVolume *m1{ (RigidbodyVolume *)m_bodies[i] };
					RigidbodyVolume* m2{ (RigidbodyVolume *)m_bodies[j] };
					result = (*m1).findCollisionFeatures( *m2 );
				}
				// If there's a collision, store the results.
				if( result.colliding ) {
					m_colliders1.push_back( m_bodies[i] );
					m_colliders2.push_back( m_bodies[j] );
					m_results.push_back( result );
				}
			}
		}
		// Sum up all forces
		for( size_t i{0}; i < bs; ++i )
			m_bodies[i]->applyForces();
		// Apply impulses to colliding objects
		for( int k{0}; k < m_impulseIteration; ++k ) {
			for( size_t i{0}; i < bs; ++i ) {
				size_t jSize{ m_results[i].contacts.size() };
				// Loop over the contact points and resolve collision by applying impulse
				for( size_t j{0}; j <jSize; ++j ) {
					RigidbodyVolume *m1{ (RigidbodyVolume *)m_colliders1[i] };
					RigidbodyVolume* m2{ (RigidbodyVolume *)m_colliders2[i] };
					(*m1).applyImpulse( *m2, m_results[i], j );
				}
			}
		}
		// Integrate forces and velocity and update position
		for( size_t i{0}; i < bs; ++i )
			m_bodies[i]->update( deltaTime );
		// Linear projection to fix sinking issues
		const size_t rs{ m_results.size() };
		for( size_t i{0}; i < rs; ++i ) {
			// Rigidbodies in the list all have volume !
			RigidbodyVolume *m1{ (RigidbodyVolume *)m_colliders1[i] };
			RigidbodyVolume* m2{ (RigidbodyVolume *)m_colliders2[i] };
			const float totalMass{ m1->getInverseMass() + m2->getInverseMass() };
			if( omath::compareFloat( totalMass, 0.0f ) )
				continue;
			// Find amount of correction. Apply slack, penetration depth and linear projection
			const float depth{ std::max( m_results[i].depth	- m_penetrationSlack, 0.0f ) };
			const float scalar{ depth / totalMass };
			const omath::vec3 correction{ m_results[i].normal * scalar * m_linearProjectionPercent };
			// Apply results
			m1->setPosition( m1->getPosition() - correction * m1->getInverseMass() );
			m2->setPosition( m2->getPosition() + correction * m2->getInverseMass() );

		}
		// Finally, solve the constraints
		for( size_t i{0}; i < bs; ++i )
			m_bodies[i]->solveConstraints( m_constraints );
	}

	void addRigidbody( Rigidbody *body ) {
		m_bodies.push_back( body );
	}

	const std::vector<Rigidbody *> &getRigidBodies() const {
		return m_bodies;
	}

	const std::vector<OBB> &getConstraints() const {
		return m_constraints;
	}

	void addConstraint( const OBB &constraint ) {
		m_constraints.push_back( constraint );
	}

	void clearRigidbodys() {
		m_bodies.clear();
	}

	void clearConstraints() {
		m_constraints.clear();
	}

private:

	std::vector<Rigidbody *> m_bodies;

	std::vector<OBB> m_constraints;

	/**
	 * To store colliding object couples
	 */
	std::vector<Rigidbody *> m_colliders1;

	std::vector<Rigidbody *> m_colliders2;

	std::vector<collisionManifold_t> m_results;

	/**
	 * How much correction to apply to collision resolution.
	 * Smaller value means deeper penetration depth allowed. Try 0.2f to 0.8f.
	 * For more sophisticated approach, see
	 * http://www.gdcvault.com/play/1020603/Physics-for-Game-Programmers-Understanding
	 */
	float m_linearProjectionPercent{ 0.45f };

	/**
	 * How deep to allow penetration. Helps control jitter. Try 0.01f to 0.1f.
	 */
	float m_penetrationSlack{ 0.01f };

	/**
	 * The solver isn't iterative but works in steps.
	 * How many times/physics frame an impulse will be applied
	 * Try 1 to 20, best 6 to 8, the more iterations, the more accurate.
	 */
	int m_impulseIteration{ 5 };

	/**
	 * Angular velocity and rotation
	 * See also: http://scienceworld.wolfram.com/physics/MomentofInertiaSphere.html
	 */
	omath::vec3 m_orientation;

	omath::vec3 m_angularVelocity;

	/**
	 *
	 */
	omath::vec3 m_sumTorques;

};

}
