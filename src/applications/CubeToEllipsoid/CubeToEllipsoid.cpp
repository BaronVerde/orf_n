
#include <applications/Camera/Camera.h>
#include <applications/CubeToEllipsoid/CubeToEllipsoid.h>
#include <base/logbook.h>
#include <geometry/EllipsoidCM.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>
#include <array>

// Start with a unit cube if no axes are given
CubeToEllipsoid::CubeToEllipsoid() : orf_n::Renderable{"CubeToEllipsoid"} {

	// Create terrain shaders
	std::vector<std::shared_ptr<orf_n::Module>> modules;
	modules.push_back( std::make_shared<orf_n::Module>( GL_VERTEX_SHADER,
			"Source/Applications/CubeToEllipsoid/CubeToEllipsoid.vert.glsl" ) );
	modules.push_back( std::make_shared<orf_n::Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/CubeToEllipsoid/CubeToEllipsoid.frag.glsl" ) );
	m_shader = new orf_n::Program{ modules };
}

CubeToEllipsoid::~CubeToEllipsoid() {
	if( m_vertexArray != nullptr )
		delete m_vertexArray;
	delete m_shader;
	//delete m_indexBuffer;
}

void CubeToEllipsoid::setup() {
	m_scene->getCamera()->setPositionAndTarget( { 2.0, 2.0, -2.0 }, { 0.0 } );
	m_scene->getCamera()->setMovementSpeed( 0.5f );
}

void CubeToEllipsoid::render() {
	m_shader->use();
	orf_n::setUniform( m_shader->getProgram(), "projectionView", m_scene->getCamera()->getViewPerspectiveMatrix() );
	m_vertexArray->bind();
	// glDrawArrays( GL_POINTS, 0, (m_tilesPerFace+1) * (m_tilesPerFace+1) );
}

void CubeToEllipsoid::cleanup() {
	;
}
