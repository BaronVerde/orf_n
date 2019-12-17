
#include "base/Logbook.h"
#include "geometry/Ellipsoid.h"
#include "geometry/Icosphere.h"
#include "geometry/OBB.h"
#include "omath/mat4.h"
#include "DrawPrimitives.h"
#include "Uniform.h"
#include "glad/glad.h"

namespace orf_n {

//static
DrawPrimitives &DrawPrimitives::getInstance() {
	static DrawPrimitives onceOnly;
	return onceOnly;
}

DrawPrimitives::~DrawPrimitives() {
	cleanupDebugDrawing();
}

void DrawPrimitives::setupDebugDrawing() {
	if( m_allSetup )
		Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::WARNING, "Debug drawing already set up." );
	else {
		// Load shaders
		std::vector<std::shared_ptr<orf_n::Module>> modules;
		modules.push_back(
				std::make_shared<Module>( GL_VERTEX_SHADER, "src/renderer/DrawPrimitives.vert.glsl" )
		);
		modules.push_back(
				std::make_shared<Module>( GL_FRAGMENT_SHADER, "src/renderer/DrawPrimitives.frag.glsl" )
		);
		m_shaderDebug = std::make_unique<Program>( modules );

		// Vertex array and indices for an aabb
		const std::vector<omath::dvec3> boxVertices {
			{ -0.5, -0.5, -0.5 },
			{  0.5, -0.5, -0.5 },
			{  0.5,  0.5, -0.5 },
			{ -0.5,  0.5, -0.5 },
			{ -0.5, -0.5,  0.5 },
			{  0.5, -0.5,  0.5 },
			{  0.5,  0.5,  0.5 },
			{ -0.5,  0.5,  0.5 }
		};
		const std::vector<GLuint> boxLoopIndices {
			0, 4, 0, 1, 5, 1, 2, 6, 2, 3, 7, 6, 5, 4, 7, 3
		};
		// bound to index 10 here because 0-7 are used by the gridmeshes
		m_boxArray = std::make_unique<VertexArray3D<omath::dvec3>>( boxVertices, 0, true );
		m_boxIndices = std::make_unique<IndexBuffer>( boxLoopIndices );
		// Here used for the center dot of the bbs @todo: make use for lines and rays
		const std::vector<omath::dvec3> lineVertices{ 2 };
		m_lineArray = std::make_unique<VertexArray3D<omath::dvec3>>( lineVertices, 0, true, orf_n::Buffer::DYNAMIC_DATA );
		// Setup buffers for a unit icosphere with 2 subdivisions. 'Nuff for debug drawing
		Icosphere is{ omath::dvec3{ 0.5, 0.5, 0.5 }, 2 };
		m_sphereArray = std::make_unique<VertexArray3D<omath::dvec3>>( is.getVertices(), 0, true );
		m_sphereIndices = std::make_unique<IndexBuffer>( is.getIndices() );
		Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO, "Debug drawing set up." );
		m_allSetup = true;
	}
}

const VertexArray3D<omath::dvec3> *DrawPrimitives::getBoxArray() const {
	return m_boxArray.get();
}

const VertexArray3D<omath::dvec3> *DrawPrimitives::getLineArray() const {
	return m_lineArray.get();
}

const IndexBuffer *DrawPrimitives::getBoxIndices() const {
	return m_boxIndices.get();
}

void DrawPrimitives::cleanupDebugDrawing() {
	if( m_allSetup ) {
		m_boxArray.reset();
		m_shaderDebug.reset();
		m_boxIndices.reset();
		m_lineArray.reset();
		m_sphereArray.reset();
		m_sphereIndices.reset();
		m_allSetup = false;
	}
}

void DrawPrimitives::drawAABB( const AABB &bb, const color_t &color ) const {
	const omath::mat4 m{ omath::mat4{ 1.0f } };
	const omath::vec3 c{ bb.getCenter() };
	const omath::mat4 modelM{ omath::translate( m, c ) * omath::scale( m, omath::vec3{ bb.getSize() } ) };
	orf_n::setUniform( m_shaderDebug->getProgram(), "modelMatrix", modelM );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", color );
	// Frame
	m_boxArray->bind();
	m_boxIndices->bind();
	glDrawElements( GL_LINE_LOOP, 16, GL_UNSIGNED_INT, NULL );
	// Center dot
	m_lineArray->bind();
	m_lineArray->updateSubData( &c[0], 3 * sizeof( GL_FLOAT ), 0 );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", orf_n::color::red );
	glDrawArrays( GL_POINTS, 1, 1 );
}

void DrawPrimitives::drawOBB( const OBB &bb, const color_t &color ) const {
	omath::mat4 id{ 1.0f };
	omath::vec3 pos{ bb.getPosition() };
	const omath::mat4 modelM{
		omath::translate( id, pos ) *
		omath::mat4{ bb.getOrientation() } *
		omath::scale( id, omath::vec3{ 2.0f * omath::vec3{ bb.getHalfSize() } } )
	};
	orf_n::setUniform( m_shaderDebug->getProgram(), "modelMatrix", modelM );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", color );
	// Shape
	m_boxArray->bind();
	m_boxIndices->bind();
	glDrawElements( GL_LINE_LOOP, 16, GL_UNSIGNED_INT, NULL );
	// Center dot
	m_lineArray->bind();
	m_lineArray->updateSubData( &pos[0], 3 * sizeof( GL_FLOAT ), 0 );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", orf_n::color::red );
	glDrawArrays( GL_POINTS, 1, 1 );
}

void DrawPrimitives::drawEllipsoid( const Ellipsoid &ellipsoid, const color_t &color ) const {
	const omath::mat4 m{ 1.0f };
	const omath::vec3 position{ ellipsoid.getPosition() };
	const omath::mat4 modelM{
		omath::translate( m, position ) * omath::scale( m, omath::vec3{ ellipsoid.getRadii() } )
	};
	orf_n::setUniform( m_shaderDebug->getProgram(), "modelMatrix", modelM );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", color );
	// Shaoe
	m_sphereArray->bind();
	m_sphereIndices->bind();
	glDrawElements( GL_LINES, m_sphereIndices->getNumber(), GL_UNSIGNED_INT, NULL );
	// Center dot
	m_lineArray->bind();
	m_lineArray->updateSubData( &position[0], 3 * sizeof( GL_FLOAT ), 0 );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", orf_n::color::red );
	glDrawArrays( GL_POINTS, 1, 1 );
}

void DrawPrimitives::drawSphere( const omath::vec3 &position,
		const float &radius, const color_t &color ) const {
	const omath::mat4 m{ omath::mat4( 1.0f ) };
	const omath::mat4 modelM{
		omath::translate( m, position ) * omath::scale( m, omath::vec3{ radius, radius, radius } )
	};
	orf_n::setUniform( m_shaderDebug->getProgram(), "modelMatrix", modelM );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", color );
	// Shape
	m_sphereArray->bind();
	m_sphereIndices->bind();
	glDrawElements( GL_LINES, m_sphereIndices->getNumber(), GL_UNSIGNED_INT, NULL );
	// Center dot
	m_lineArray->bind();
	m_lineArray->updateSubData( &position[0], 3 * sizeof( GL_FLOAT ), 0 );
	orf_n::setUniform( m_shaderDebug->getProgram(), "debugColor", orf_n::color::red );
	glDrawArrays( GL_POINTS, 1, 1 );
}

const Program *DrawPrimitives::getProgramPtr() const {
	return m_shaderDebug.get();
}

}
