
// A particle has mass but not volume and is affected by a single force

#pragma once

#include <geometry/Line.h>
#include <geometry/Ray.h>
#include <physics/Rigidbody.h>

namespace orf_n {

class Particle : public Rigidbody {
public:
	Particle( const omath::vec3 &position ) :
		m_position{position}, m_mass{1.0f}, m_bounce{0.7f},
		m_gravity{ 0.0f, -9.82f, 0.0f }, m_friction{0.95f} {}

	virtual ~Particle() {}

	// Integrate position over time
	virtual void update( const float deltaTime ) override {
		// Euler or verlet integration ?
		if( USE_EULER ) {
			m_oldPosition = m_position;
			omath::vec3 acceleration{ m_forces * ( 1.0f / m_mass ) };
			omath::vec3 oldVelocity{ m_velocity };
			m_velocity = m_velocity * m_friction + acceleration * deltaTime;
			// Taking the velocity median reduces error subsummation over time.
			m_position += ( oldVelocity + m_velocity ) * 0.5f * deltaTime;
		} else {
			// Use verlet integration (implicit velocity)
			omath::vec3 velocity = m_position - m_oldPosition;
			m_oldPosition = m_position;
			float deltaSquare = deltaTime * deltaTime;
			// Position by integration
			m_position += velocity * m_friction + m_forces * deltaSquare;
		}
	}

	virtual void applyForces() override {
		// Only one force for now
		m_forces = m_gravity;
	}

	virtual void solveConstraints( const std::vector<OBB> &constraints ) override {
		size_t size{ constraints.size() };
		for( size_t i{0}; i < size; ++i ) {
			// Line for the path the particle has travelled in between, to avoid tunnelling
			Line travelled( m_oldPosition, m_position );
			// If particle collided create a ray to find intersection
			if( travelled.testOBB( constraints[i] ) ) {
				if( USE_EULER ) {
					omath::vec3 direction{ omath::normalize( m_velocity ) };
					Ray ray{ m_oldPosition, direction };
					raycastResult_t result;
					if( ray.raycast( constraints[i], &result ) ) {
						// Move particle a little bit above the collision point to allow it to roll down a slope
						m_position = result.point + result.normal * 0.002f;
						// Deconstruct velocity into parallel and perpendicular components
						// relative to collision normal, conserve momentum
						omath::vec3 vn{ result.normal * omath::dot( result.normal, m_velocity ) };
						omath::vec3 vt{ m_velocity - vn };
						// To avoid tunnelling record where it came from
						m_oldPosition = m_position;
						m_velocity = vt - vn * m_bounce;
						// @todo break to solve only one constraint, evtl. outcomment
						break;
					}
				} else {
					// Use verlet integration (implicit velocity)
					omath::vec3 velocity = m_position - m_oldPosition;
					omath::vec3 direction = omath::normalize( velocity );
					Ray ray( m_oldPosition, direction );
					raycastResult_t result;
					if( ray.raycast( constraints[i], &result ) ) {
						// Same as above
						m_position = result.point + result.normal * 0.002f;
						omath::vec3 vn{ result.normal * omath::dot( result.normal, velocity ) };
						omath::vec3 vt{ velocity - vn };
						m_oldPosition = m_position - ( vt - vn * m_bounce );
						break;
					}
				}
			}
		}
	}

	void setPosition( const omath::vec3 &pos ) {
		m_position = m_oldPosition = pos;
	}

	const omath::vec3 &getPosition() const {
		return m_position;
	}

	void setBounce( const float b ) {
		m_bounce = b;
	}

	const float &getBounce() const {
		return m_bounce;
	}

private:
	static constexpr bool USE_EULER{ true };

	/* @todo Some members could be made global or moved to a material or so
	 * visual represenattion for rendering: a mesh or so. For now a sphere is
	 * hardcoded */
	omath::vec3 m_position{ 0.0f, 0.0f, 0.0f };

	omath::vec3 m_oldPosition;

	omath::vec3 m_forces;

	omath::vec3 m_velocity{ 0.0f, 0.0f, 0.0f };

	float m_mass{ 1.0f };

	// 1 - this is lost when the particle bounces back
	float m_bounce{ 0.7f };

	omath::vec3 m_gravity{ 0.0f, -9.82f, 0.0f };

	float m_friction{ 0.95f };

};

}
