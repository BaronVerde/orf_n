
#include "HydroTile.h"
#include <iostream>
#include "omath/common.h"
#include "renderer/Sampler.h"
#include "renderer/Module.h"
#include "base/Globals.h"
#include "scene/Scene.h"
#include "renderer/Uniform.h"
#include <png++/image.hpp>
#include <png++/gray_pixel.hpp>

HydroTile::HydroTile() : orf_n::Renderable{ "HydroTile" } {
	if( m_extent == 0 || m_extent > 4096 || !omath::isPowerOf2( m_extent ) )
		std::cerr << "Extent must be > 0 and <= 4096 !" << std::endl;
	m_gridMesh = std::make_unique<terrain::GridMesh>( m_extent );
	// Creating the liftrate map will be done elsewhere later
	if( !omath::isPowerOf2( m_extent ) || m_extent < 256 || m_extent > 4096 )
		std::cout << "Extent of the patch must be power of 2 and between 256 and 4096" << std::endl;
	size_t numPoints{ m_profileEW.size() };
	if( numPoints >= m_extent || !omath::isPowerOf2( numPoints ) || numPoints != m_profileNS.size() )
		std::cerr << "Lists of control points must be equal in size, power of 2 and smaller than the extent" << std::endl;
	// create profile texture for lift rate
	png::image<png::gray_pixel> image( static_cast<uint32_t>(numPoints), static_cast<uint32_t>(numPoints) );
	for( size_t i{1}; i < numPoints-2; ++i ) {
		for( size_t j{0}; j < numPoints; ++j ) {
			//const float valuef{ omath::lerp( m_profileNS[i], m_profileEW[j], 0.5f ) };
			float valuef{ omath::cubicInterpolate( m_profileNS[i-1], m_profileNS[i], m_profileNS[i+1], m_profileNS[i+2], 0.5f ) };
			valuef *= m_profileEW[j];
			const uint8_t value{ static_cast<uint8_t>( std::round( valuef * 100.0f ) ) };
			image[i][j] = png::gray_pixel( value );
		}
	}
	image.write( m_liftrateFilename );
	// Build shader programs
	std::vector<std::shared_ptr<orf_n::Module>> modules;
	modules.push_back( std::make_shared<orf_n::Module>( GL_VERTEX_SHADER,
			"src/applications/TerrainSim/displacement.vert.glsl" ) );
	modules.push_back( std::make_shared<orf_n::Module>( GL_FRAGMENT_SHADER,
			"src/applications/TerrainSim/displacement.frag.glsl" ) );
	m_program = std::make_unique<orf_n::Program>( modules );
}

HydroTile::~HydroTile() {}

void HydroTile::setup() {
	// load the liftrate texture
	const float fextent{ static_cast<float>(m_extent) };
	m_liftrateTexture = std::make_unique<orf_n::Texture2D>( m_liftrateFilename, 1 );
	m_liftrateTexture->bindToUnit();
	orf_n::setDefaultSampler( m_liftrateTexture->getName(), orf_n::LINEAR_CLAMP );
	m_scene->getCamera()->setPositionAndTarget( { -0.07f, 0.105f, -0.08f }, { 0.0f, 0.0f, 1.0f } );
	m_scene->getCamera()->setMovementSpeed( 0.1f );
	m_scene->getCamera()->setNearPlane( 0.01f );
	m_scene->getCamera()->setFarPlane( 10.0f );
	m_program->use();
	orf_n::setUniform( m_program->getProgram(), "u_posToLiftrateMap", omath::vec2{ fextent / 8.0f, fextent / 8.0f } );
	orf_n::setUniform( m_program->getProgram(), "u_liftrate", m_liftrate );
	orf_n::setUniform( m_program->getProgram(), "u_timeStep", m_timeStep );
}

void HydroTile::render() {
	m_gridMesh->bind();
	m_program->use();
	float oldTimeStep{ m_timeStep };
	if( orf_n::globals::showAppUI )
		ImGui::SliderFloat( "#time steps", &m_timeStep, 1.0f, 100.0f );
	if( m_timeStep != oldTimeStep )
		orf_n::setUniform( m_program->getProgram(), "u_timeStep", m_timeStep );
	orf_n::setCameraPosition( omath::vec3{ m_scene->getCamera()->getPosition() } );
	omath::mat4 mvpMatrix{ m_scene->getCamera()->getPerspectiveMatrix() *
		m_scene->getCamera()->getViewMatrix() * m_modelMatrix };
	orf_n::setModelViewProjectionMatrix( mvpMatrix );
	glEnable( GL_DEPTH_TEST );
	const GLenum mode = m_scene->getCamera()->getWireframeMode() ? GL_LINE : GL_FILL;
	glPolygonMode( GL_FRONT_AND_BACK, mode );
	glDrawElements( GL_TRIANGLES, m_gridMesh->getNumberOfIndices(), GL_UNSIGNED_INT, NULL );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void HydroTile::cleanup() {}
