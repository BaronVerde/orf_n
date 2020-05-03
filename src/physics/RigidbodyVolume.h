
/**
 * Rigidbody derived class for rigidbodies with mass and volume.
 * Holds members vars for Euler integration.
 */

#pragma once

#include <base/globals.h>
#include <geometry/ellipsoid.h>
#include <geometry/OBB.h>
#include <geometry/Sphere.h>
#include <omath/vec3.h>
#include <physics/CollisionFeatures.h>
#include <physics/Rigidbody.h>
#include <renderer/DrawPrimitives.h>

namespace orf_n {

class RigidbodyVolume : public Rigidbody {
public:
	RigidbodyVolume() : Rigidbody() {}

	RigidbodyVolume( const rigidbody_t type ) : Rigidbody() {
		m_type = type;
	}

	virtual ~RigidbodyVolume() {}

	void setPosition( const omath::vec3 &position ) {
		m_position = position;
	}

	const omath::vec3 &getPosition() const {
		return m_position;
	}

	/**
	 * Just call the primitive draw functions.
	 */
	virtual void render( const DrawPrimitives *const drawPrimitives ) override {
		synchCollisionVolumes();
		if( m_type == SPHERE ) {
			drawPrimitives->getInstance().drawSphere( m_sphere.getPosition(), m_sphere.getRadius(),
					orf_n::color::green );
		} else if( m_type == BOX )
			drawPrimitives->getInstance().drawOBB( m_box, orf_n::color::green );
	}

	virtual void update( const float deltaTime ) override {
		const float damping{ 0.98f };
		const omath::vec3 acceleration{ m_forces * getInverseMass() };
		m_linearVelocity += acceleration * deltaTime;
		m_linearVelocity *= damping;
		m_position += m_linearVelocity * deltaTime;
		synchCollisionVolumes();
	}

	virtual void applyForces() override {
		m_forces = m_gravity * m_mass;
	}

	/**
	 * Keep the position of the bodie's representations in sync with the position.
	 */
	void synchCollisionVolumes() {
		m_sphere.setPosition( m_position );
		m_box.setPosition( m_position );
	}

	/**
	 * Returns inverse mass or 0 if object has no mass. 0 means "infinity" mass,
	 * object is unmoveable.
	 */
	float getInverseMass() const {
		if( omath::compareFloat( m_mass, 0.0f ) )
			return 0.0f;
		else
			return 1.0f / m_mass;
	}

	/**
	 * An impulse i applied immediately. Forces otoh act over time.
	 */
	void addLinearImpulse( const omath::vec3 &impulse ) {
		m_linearVelocity += impulse;
	}

	/**
	 * Returns the collision manifold between thi and another voluminous rigidbody
	 */
	collisionManifold_t findCollisionFeatures( const RigidbodyVolume &other ) const {
		collisionManifold_t result;
		// Find features by type
		if( m_type == SPHERE ) {
			if( other.m_type == SPHERE )
				result = orf_n::findCollisionFeatures( m_sphere, other.m_sphere );
			else if( other.m_type == BOX)
				result = orf_n::findCollisionFeatures( other.m_box, m_sphere );
		} else if( m_type == BOX ) {
			if( other.m_type == SPHERE )
				result = orf_n::findCollisionFeatures( m_box, other.m_sphere );
			else if( other.m_type == BOX )
				result = orf_n::findCollisionFeatures( m_box, other.m_box );
		}
		return result;
	}

	/**
	 * Apply impulse from collision to both bodies. Include friction.
	 */
	void applyImpulse( RigidbodyVolume &b, const collisionManifold_t &manifold, size_t c ) {
		// Linear Velocity
		const float invMass1{ getInverseMass() };
		const float invMass2{ b.getInverseMass() };
		const float invMassSum{ invMass1 + invMass2 };
		if( omath::compareFloat( invMassSum, 0.0f ) )
			return;
		// Relative velocity
		const omath::vec3 relativeVel{ b.m_linearVelocity - m_linearVelocity };
		// Relative collision normal
		const omath::vec3 relativeNorm{ omath::normalize( manifold.normal ) };
		// Early out: are they moving away from each other ?
		if( omath::dot( relativeVel, relativeNorm ) > 0.0f )
			return;
		// Find j: magnitude of impulse needed to resolve the collision (smaller value)
		const float e{ std::min( m_coeffOfRestitution, b.m_coeffOfRestitution ) };
		float numerator{ -( 1.0f + e ) * omath::dot( relativeVel, relativeNorm ) };
		float j{ numerator / invMassSum };
		if( manifold.contacts.size() > 0 && j != 0.0f )
			j /= (float)manifold.contacts.size();
		// Apply the impulse by modifying velocity directly
		const omath::vec3 impulse{ relativeNorm * j };
		m_linearVelocity -= impulse * invMass1;
		b.m_linearVelocity += impulse * invMass2;
		// Friction. First find tangent to the collision normal
		const omath::vec3 t{ relativeVel - ( relativeNorm * omath::dot( relativeVel, relativeNorm ) ) };
		if( omath::compareFloat( omath::magnitudeSq( t ), 0.0f ) )
			return;
		const omath::vec3 tn{ omath::normalize(t) };
		// Find jt, the magnitude of friction
		numerator = -omath::dot( relativeVel, t );
		float jt{ numerator / invMassSum };
		if( manifold.contacts.size() > 0 && jt != 0.0f )
			jt /= (float)manifold.contacts.size();
		if( omath::compareFloat( jt, 0.0f ) )
			return;
		// Clamp friction according to Coulomb's law
		float friction{ std::sqrt( m_friction * b.m_friction ) };
		if( jt > j * friction )
			jt = j * friction;
		else if( jt < -j * friction )
			jt = -j * friction;
		// Apply tangential impulse (friction) to the bodie's velocities
		const omath::vec3 tangentImpulse{ t * jt };
		m_linearVelocity -= tangentImpulse * invMass1;
		b.m_linearVelocity += tangentImpulse * invMass2;
	}

	/**
	 * get the inverse inertia tensor for the shape
	 */
	omath::mat4 getInverseTensor() {
		// main diagonal;
		float ix{0.0f}, iy{0.0f}, iz{0.0f}, iw{0.0f};
		if( m_mass != 0 && m_type == SPHERE ) {
			const float r2{ m_sphere.getRadius() * m_sphere.getRadius() };
			ix = r2 * m_mass * SPHERE_FRACTION;
			iy = r2 * m_mass * SPHERE_FRACTION;
			iz = r2 * m_mass * SPHERE_FRACTION;
			iw = 1.0f;
		} else if( m_mass != 0 && m_type == BOX ) {
			const omath::vec3 size = m_box.getHalfSize() * 2.0f;
			const float x2{ size.x * size.x };
			const float y2{ size.y * size.y };
			const float z2{ size.z * size.z };
			ix = ( y2 + z2 ) * m_mass * BOX_FRACTION;
			iy = ( x2 + z2 ) * m_mass * BOX_FRACTION;
			iz = ( x2 + y2 ) * m_mass * BOX_FRACTION;
			iw = 1.0f;
		}
		return omath::inverse(
				omath::mat4{ ix, 0, 0, 0,
						   0, iy, 0, 0,
						   0, 0, iz, 0,
						   0, 0, 0, iw } );
	}

	/**
	 *
	 */
	virtual void addRotationalImpulse( const omath::vec3 &point, const omath::vec3 &impulse );

private:

	static constexpr float SPHERE_FRACTION{ 2.0f / 5.0f };

	static constexpr float BOX_FRACTION{ 1.0f / 12.0f };

	omath::vec3 m_position;

	/**
	 * Linear velocity without rotation. e.g. influenced by gravity.
	 */
	omath::vec3 m_linearVelocity;

	/**
	 * Gravity force the body experiences.
	 */
	omath::vec3 m_gravity{ 0.0f, -9.81f, 0.0f };

	/**
	 * Sum of all forces
	 */
	omath::vec3 m_forces;

	/**
	 * @todo: this could be packed into a material class
	 */
	float m_mass{ 1.0f };

	float m_coeffOfRestitution{ 0.5f };

	float m_friction{ 0.6f };

	/**
	 * Representations of the bodie's volume, either a sphere or a box.
	 */
	OBB m_box;

	Sphere m_sphere;

};

}
