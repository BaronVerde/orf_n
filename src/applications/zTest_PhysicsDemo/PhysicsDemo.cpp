
#include <applications/Camera/Camera.h>
#include <applications/zTest_PhysicsDemo/PhysicsDemo.h>
#include <geometry/AABB.h>
#include <geometry/Ellipsoid.h>
#include <GL/glew.h>
#include <omath/mat4.h>
#include <physics/RigidbodyVolume.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>
//#include "omath/gtc/random.hpp"

PhysicsDemo::PhysicsDemo() : orf_n::Renderable{ "PhysicsDemo" } {}

PhysicsDemo::~PhysicsDemo() {}

void PhysicsDemo::setup() {
	orf_n::DrawPrimitives::getInstance().setupDebugDrawing();
	m_scene->getCamera()->setPositionAndTarget(
			omath::dvec3{ 18.0, 8.5, 6.3 },
			omath::dvec3{ 0.0, 2.0, -1.0 }
	);
	m_scene->getCamera()->setNearPlane( 0.1f );
	m_scene->getCamera()->setFarPlane( 100.0f );
	m_scene->getCamera()->calculateFOV();
	m_scene->getCamera()->setMovementSpeed( 10.0f );

	const orf_n::OBB ground{
		omath::vec3{ 0.0f, 0.0f, 0.0f },
		omath::vec3{ 10.0f, 0.15f, 10.0f },
		omath::mat3{ 1.0f }
	};

	omath::mat4 id{ 1.0f };
	omath::mat4 rotM{
		omath::rotate( id, omath::radians( 30.716f ), omath::vec3{ 1.0f, 0.0f, 0.0f } )
	};
	const orf_n::OBB obb1{
		omath::vec3{ 0.0f, 1.86f, 0.0f },
		omath::vec3{ 2.0f, 0.15f, 2.0f },
		omath::mat3{ rotM }
	};

	rotM =	omath::rotate( id, omath::radians( -24.233f ), omath::vec3{ 0.0f, 1.0f, 0.0f } ) *
			omath::rotate( id, omath::radians( 9.128f ), omath::vec3{ 0.0f, 0.0f, 1.0f } ) *
			omath::rotate( id, omath::radians( -33.964f ), omath::vec3{ 1.0f, 0.0f, 0.0f } );
	const orf_n::OBB obb2{
		omath::vec3{ -3.0f, 4.6f, 0.0f },
		omath::vec3{ 2.0f, 0.15f, 2.0f },
		omath::mat3{ rotM }
	};

	rotM = omath::rotate( id, omath::radians( 24.702f ), omath::vec3{ 1.0f, 0.0f, 0.0f } );
	const orf_n::OBB obb3{
		omath::vec3{ 1.0f, 4.93f, 0.0f },
		omath::vec3{ 2.0f, 0.15f, 0.7817011f },
		rotM
	};

	m_system.addConstraint( ground );
	m_system.addConstraint( obb1 );
	m_system.addConstraint( obb2 );
	m_system.addConstraint( obb3 );

	// Place num particles randomly
	for( uint32_t i{0}; i < NUM_PARTCLES; ++i ) {
		// @todo ! omath::vec2 pos{ omath::diskRand( 2.0f ) };
		// orf_n::Particle p{ omath::vec3{ pos.x, 6.0f, pos.y } };
		//m_system.addRigidbody( p );
	}

}

void PhysicsDemo::render() {
	glEnable( GL_DEPTH_TEST );
	glPointSize( 3.0f );
	const orf_n::Program *p{ m_drawPrimitives.getProgramPtr() };
	p->use();
	orf_n::setUniform( p->getProgram(), "projViewMatrix", m_scene->getCamera()->getViewPerspectiveMatrix() );
	orf_n::AABB box{ omath::dvec3{ -0.5, -0.5, -0.5 }, omath::dvec3{ 0.5, 0.5, 0.5 } };
	omath::dvec3 s{ box.getSize() };
	for( size_t i{0}; i <= m_system.getConstraints().size(); ++i )
		m_drawPrimitives.drawOBB( m_system.getConstraints()[i], orf_n::color::red );

	/*orf_n::Ellipsoid sphere{ omath::vec3{ 1.0f, 1.0f, 1.0f } };
	sphere.setPosition( omath::vec3{ 2.0f, 0.0f, 0.0f } );
	m_drawPrimitives.drawEllipsoid( sphere, orf_n::color::blue );*/
	glPointSize( 1.0f );
}

void PhysicsDemo::cleanup() {}


/*
 * #include <iostream>
#include <chrono>
#include <thread>

int main()
{
    using namespace std::chrono;
    using Framerate = duration<steady_clock::rep, std::ratio<1, 60>>;
    auto next = steady_clock::now() + Framerate{1};
    int i = 0;
    while(true)
    {
        std::cout << ++i << std::endl;
        //update();
        std::this_thread::sleep_until(next);
        next += Framerate{1};
        //render();
    }
    return 0;
}
 *
 */
