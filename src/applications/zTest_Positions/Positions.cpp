
#include <applications/Camera/Camera.h>
#include <applications/zTest_Positions/Positions.h>
#include <geometry/AABB.h>
#include <geometry/Geodetic.h>
#include <omath/mat4.h>
#include <renderer/Color.h>
#include <renderer/Texture2D.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>

Positions::Positions() : orf_n::Renderable( "Positions" ) {
	m_ellipsoid = std::make_unique<orf_n::Ellipsoid>( orf_n::Ellipsoid::WGS84_ELLIPSOID );
	m_gridMesh = std::make_unique<terrain::GridMesh>( 2048 );
	m_terrainTile = std::make_unique<terrain::TerrainTile>( TERRAIN_FILES[1], m_ellipsoid.get() );

	std::vector<std::shared_ptr<orf_n::Module>> modules;
	modules.push_back( std::make_shared<orf_n::Module>( GL_VERTEX_SHADER,
			"Source/Applications/Positions/Positions.vert.glsl" ) );
	modules.push_back( std::make_shared<orf_n::Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/Positions/Positions.frag.glsl" ) );
	m_program = std::make_unique<orf_n::Program>( modules );
}

Positions::~Positions() {}

void Positions::setup() {
	m_program->use();
	const omath::dvec3 camPos{
		m_ellipsoid->toCartesian(
				orf_n::Geodetic{ omath::radians( m_terrainTile->getLatLon().x ),
								 omath::radians( m_terrainTile->getLatLon().y ), 0.0 }
		)
	};
	const omath::dvec3 target{ m_terrainTile->getWorldCenter() };
	m_scene->getCamera()->setPositionAndTarget( camPos, target );
	//m_scene->getCamera()->setPositionAndTarget( { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } );
	m_scene->getCamera()->setMovementSpeed( 5000.0f );
	m_scene->getCamera()->setNearPlane( 10.0f );
	m_scene->getCamera()->setFarPlane( 10000000.0f );

	m_drawPrimitives.setupDebugDrawing();
}

void Positions::render() {
	m_program->use();
	m_gridMesh->bind();
	//m_terrainTile->m_positionsArray->bind();
	m_terrainTile->getPositionsHigh()->bindToUnit();
	m_terrainTile->getPositionsLow()->bindToUnit();
	orf_n::Camera *cam{ m_scene->getCamera() };
	const omath::dmat4 view{ omath::lookAt(
			cam->getPosition(),
			cam->getPosition() + omath::dvec3{ cam->getFront() },
			omath::dvec3{ cam->getUp() } )
	};
	// Identity matrix as model matrix
	const omath::dmat4 mv{ view * omath::dmat4{ 1.0 } };
	// this matrix can be used for all objects in same coord system
	const omath::mat4 mvRTE{
		omath::vec4{ mv[0] },
		omath::vec4{ mv[1] },
		omath::vec4{ mv[2] },
		omath::vec4{ 0.0f, 0.0f, 0.0f, static_cast<float>( mv[3][3] ) }
	};
	orf_n::setModelViewProjectionMatrixRTE( cam->getPerspectiveMatrix() * mvRTE );
	orf_n::setViewProjectionMatrix( cam->getViewPerspectiveMatrix() );
	orf_n::setCameraPosition( cam->getPosition() );
	orf_n::setUniform( m_program->getProgram(), "u_drawColor", orf_n::color::green );
	glDrawElements( GL_POINTS, m_gridMesh->getNumberOfIndices(), GL_UNSIGNED_INT, (const void *)0 );
	//glDrawArrays( GL_POINTS, 0, 2048 * 2048 );

	// Bounding box
	const orf_n::Program *p{ m_drawPrimitives.getProgramPtr() };
	p->use();
	orf_n::setUniform( p->getProgram(), "projViewMatrix", m_scene->getCamera()->getViewPerspectiveMatrix() );
	glPointSize( 3.0f );
	m_drawPrimitives.drawAABB( *( m_terrainTile->getAABB() ), orf_n::color::white );

}

void Positions::cleanup() {
	m_drawPrimitives.cleanupDebugDrawing();
}
