
#include <base/logbook.h>
#include "applications/Camera/Camera.h"
//#include "applications/CubeToEllipsoid/CubeToEllipsoid.h"
//#include "applications/IcosphereEllipsoid/IcosphereEllipsoid.h"
#include "applications/SkyBox/SkyBox.h"
#include "applications/TerrainLOD/TerrainLOD.h"
//#include "applications/TerrainSim/HydroTile.h"
//#include "applications/TerrainErosion/HydroErosionMDH07.h"
#include "base/Globals.h"	// deltaTime
#include "renderer/Renderer.h"
//#include "Applications/PhysicsDemo/PhysicsDemo.h"

namespace orf_n {

extern double globals::deltaTime;

Renderer::Renderer( bool debug ) : m_debug{ debug } {}

Renderer::~Renderer() {}

void Renderer::setupRenderer() {
	// create render window, camera object and basic UIOverlay
	m_window = new GlfwWindow( "orf-n", 1600, 1000, m_debug );
	// Before we proceed, perform a basic look around.
	if( !checkEnvironment() ) {
		std::string s{ "The graphics environment is unfit to run this program." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error{ s };
	}
	// Just to have a camera object. Applications may set position and target
	m_camera = new Camera{ m_window, omath::dvec3{ 0.0, 0.0, 1.0 }, omath::dvec3{ 0.0, 0.0, 0.0 },
		omath::vec3{ 0.0f, 1.0f, 0.0f }, 1.0f, 1000.0f, Camera::FIRST_PERSON };
	m_overlay = new UIOverlay( m_window );

	m_framebuffer = new orf_n::Framebuffer( m_window->getWidth(), m_window->getHeight() );
	m_framebuffer->addColorAttachment( GL_SRGB, GL_COLOR_ATTACHMENT0 );
	m_framebuffer->addDepthAttachment( GL_DEPTH_COMPONENT32F );
	if( !m_framebuffer->isComplete() ) {
		std::string s{ "Error creating framebuffer." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error{ s };
	} else
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, "Framebuffer/renderbuffer created." );

	// Build the scene and set it up.
	m_scene = new Scene( m_window, m_camera, m_overlay );
	//m_scene->addRenderable( 1, std::make_shared<CubeToEllipsoid>( 2, omath::dvec3{1.0} ) );
	//m_scene->addRenderable( 1, std::make_shared<IcosphereEllipsoid>( Ellipsoid::WGS84_ELLIPSOID, 7 ) );
	m_scene->addRenderable( 1, std::make_shared<TerrainLOD>() );
	//m_scene->addRenderable( 1, std::make_shared<Positions>() );
	//m_scene->addRenderable( 1, std::make_shared<HydroErosionMDH07>() );
	//m_scene->addRenderable( 1, std::make_shared<PhysicsDemo>() );
	m_scene->addRenderable( 9, std::make_shared<SkyBox>() );

}

void Renderer::setup() const {
	m_scene->setup();
}

void Renderer::render() const {
	logbook::log_msg( orf_n::logbook::RENDERER, orf_n::logbook::INFO,
			"--- Entering main loop ---" );
	double lastFrame { 0.0 };
	uint64_t frameCounter { 0 };

	while( !glfwWindowShouldClose( m_window->getWindow() ) ) {
		double currentFrame { glfwGetTime() };
		++frameCounter;
		globals::deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//glEnable( GL_FRAMEBUFFER_SRGB );
		// Render in the framebuffer first
		m_framebuffer->bind( GL_DRAW_FRAMEBUFFER );
		m_framebuffer->clear( omath::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } );

		m_scene->prepareFrame();
		// Called after prepareFrame() because UIOverlay has to start a new frame.
		m_scene->getCamera()->updateMoving();

		m_scene->render();
		m_scene->endFrame();
		//glDisable( GL_FRAMEBUFFER_SRGB );

		// Blit framebuffer to default window framebuffer
		m_framebuffer->bind( GL_READ_FRAMEBUFFER );
		// or m_framebuffer->unbind()
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
		// clear to blue to distinguish between draw framebuffer; color::blue
		glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glBlitFramebuffer( 0, 0, m_window->getWidth(), m_window->getHeight(),
						   0, 0, m_window->getWidth(), m_window->getHeight(),
						   GL_COLOR_BUFFER_BIT, GL_NEAREST );

		glfwPollEvents();
		glfwSwapBuffers( m_scene->getWindow()->getWindow() );

	}
	logbook::log_msg( orf_n::logbook::RENDERER, orf_n::logbook::INFO,
			"--- Leaving main loop ---" );
}

void Renderer::cleanup() const {
	m_scene->cleanup();
}

void Renderer::cleanupRenderer() {
	delete m_framebuffer;
	delete m_scene;
	delete m_overlay;
	delete m_camera;
	delete m_window;
}

// static
bool Renderer::checkEnvironment() {
	GLint retval;
	glGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &retval );
	std::string s{ "Maximum combined texture units: " + std::to_string( retval ) + '.' };
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &retval );
	s = "Maximum texture size: " + std::to_string( retval ) + '.';
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
	glGetIntegerv( GL_MAX_TEXTURE_BUFFER_SIZE, &retval );
	s = "Maximum texture buffer size: " + std::to_string( retval ) + '.';
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
	return true;
}

}	// namespace
