
#include <applications/IcosphereEllipsoid/zBackup_single_precision/IcosphereEllipsoid.h>
#include <base/Globals.h>	// deltaTime
#include <base/Logbook.h>
#include <geometry/ViewFrustum.h>
#include <omath/mat4.h>
#include <renderer/Sampler.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>
#include <iostream>
#include "GL/glew.h"
#include <memory>

using namespace orf_n;

IcosphereEllipsoid::IcosphereEllipsoid( GlfwWindow *win,
										const omath::vec3 &axes,
										const uint32_t numSubDivs ) :
				Renderable{ "IcosphereEllipsoid" },
				m_axes{ axes },
				m_numSubDivs{ numSubDivs } {}

IcosphereEllipsoid::~IcosphereEllipsoid() {
	Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO,
			"IcosphereEllipsoid cleaned up and deleted." );
}

void IcosphereEllipsoid::setup() {
	m_numSubDivs = 6;
	// Create the shaders
	std::vector<std::shared_ptr<Module>> modules;
	modules.push_back( std::make_shared<Module>( GL_VERTEX_SHADER,
			"Source/Applications/IcosphereEllipsoid/IcosphereEllipsoid.vert.glsl" ) );
	modules.push_back( std::make_shared<Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/IcosphereEllipsoid/IcosphereEllipsoid.frag.glsl" ) );
	m_shader = new Program( modules );

	//m_texture = new Texture2D{ "Resources/Textures/Ellipsoids/EarthDay_4k.jpg", 0, true };
	m_texture = new Texture2D{ "Resources/Textures/Ellipsoids/sphere1.png", 0, true };
	setDefaultSampler( m_texture->getName(), LINEAR_CLAMP );

	m_ico = new Icosphere{ m_axes, m_numSubDivs };
	// Prepare vertex array; double values to two floats interleaved
	std::vector<omath::vec3> verts( m_ico->getVertices().size() * 2 );
	for( size_t i{0}; i < m_ico->getVertices().size(); ++i ) {
		doubleToTwoFloats( m_ico->getVertices()[i], verts[i*2], verts[i*2+1] );
	}
	m_vertexArray = new VertexArray3D<omath::vec3>{ verts, 0, true };
	m_indexBuffer = new IndexBuffer{ m_ico->getIndices() };
	omath::mat4 m{ 1.0f };
	m_modelMatrix = omath::translate( m, { 0.0f, 0.0f, 0.0f } ) *
			omath::rotate( m, -90.0f, { 1.0f, 0.0f, 0.0f } );
	m_scene->getCamera()->setPositionAndTarget( { 0.0, 0.0, -7000000.0 }, { 0.0, 0.0, 0.0 } );
	m_scene->getCamera()->setNearPlane( 0.01f );
	m_scene->getCamera()->setFarPlane( 10000000.0f );
	m_scene->getCamera()->setMovementSpeed( 3000.0f );
	m_scene->getCamera()->calculateFOV();
	m_shader->use();
	//setUniform( m_shader->getProgram(), "u_modelMatrix", m_modelMatrix );
	setUniform( m_shader->getProgram(), "u_oneOverRadiiSquared",
			static_cast<omath::vec3>( m_ico->getOneOverRadiiSquared() ) );
	setUniform( m_shader->getProgram(), "u_showDisplacement", m_showDisplacement );
	setUniform( m_shader->getProgram(), "u_normalType", m_normalType );

	Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO, "IcosphereEllipsoid created and set up." );
}

void IcosphereEllipsoid::render() {
	float nearPlane{ m_scene->getCamera()->getNearPlane() };
	float farPlane{ m_scene->getCamera()->getFarPlane() };
	const bool oldShowDisplacement{ m_showDisplacement };
	const int oldNormalType{ m_normalType };
	const omath::ivec2 oldMapDimension{ m_mapDimension };
	int clicked{0};
	if( globals::showAppUI ) {
		ImGui::Begin( "Ellipsoid Params" );
		ImGui::Checkbox( "Origin in frustum: ", &m_visible );
		ImGui::SliderFloat( "Near plane", &nearPlane, 0.01f, 100.0f );
		ImGui::SliderFloat( "Far plane", &farPlane, 10.0f, 10000000.0f );
		ImGui::Checkbox( "Height displacement", &m_showDisplacement );
		// Cut out 1
		ImGui::Text( "Calculate normals:" );
		ImGui::RadioButton( "centric", &m_normalType, 0 ); ImGui::SameLine();
		ImGui::RadioButton( "detic", &m_normalType, 1 ); ImGui::SameLine();
		ImGui::RadioButton( "combined", &m_normalType, 2 );
		if( ImGui::Button( "Calculate Height Map" ) )
			clicked++;
		ImGui::End();
	}
	// Cut out 2
	if( nearPlane != m_scene->getCamera()->getNearPlane() )
		m_scene->getCamera()->setNearPlane( nearPlane );
	if( farPlane != m_scene->getCamera()->getFarPlane() )
		m_scene->getCamera()->setFarPlane( farPlane );
	m_shader->use();
	if( oldNormalType != m_normalType )
		setUniform( m_shader->getProgram(), "u_normalType", m_normalType );
	if( oldMapDimension != m_mapDimension )
		std::cout << "Map dimension changed " << m_mapDimension.x << '/' << m_mapDimension.y << std::endl;
	if( m_showDisplacement != oldShowDisplacement ) {
		setUniform( m_shader->getProgram(), "u_showDisplacement", m_showDisplacement );
		std::cout << m_showDisplacement << std::endl;
	}

	m_visible = true;
	/*m_visible = ( OUTSIDE != m_scene->getCamera()->getViewFrustum().isSphereInFrustum(
			omath::vec3( 0.0f, 0.0f, 0.0f ), (float)m_ico->getRadii().x ) );*/
	if( m_visible ) {
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_CULL_FACE );
		glFrontFace( GL_CCW );
		glCullFace( GL_BACK );
		m_vertexArray->bind();
		m_indexBuffer->bind();
		omath::mat4 vpMatrix{ m_scene->getCamera()->getViewProjMatrix() * m_modelMatrix };
		setViewProjectionMatrix( vpMatrix );
		const GLenum mode = m_scene->getCamera()->getWireframeMode() ? GL_LINE : GL_FILL;
		glPolygonMode( GL_FRONT_AND_BACK, mode );
		glDrawElements( GL_TRIANGLES, m_indexBuffer->getNumber(), GL_UNSIGNED_INT, NULL );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
}

void IcosphereEllipsoid::cleanup() {
	delete m_texture;
	delete m_vertexArray;
	delete m_shader;
	delete m_indexBuffer;
	delete m_ico;
}

// Cut out
/*ImGui::Separator();
		ImGui::Checkbox( "Perlin Module active", &m_perlinActive );
		ImGui::InputInt( "Octave count (4-30)", &m_perlinOctaveCount );
		if( ImGui::IsItemHovered() )
			ImGui::SetTooltip( "Number of octaves; 4-30; the more, the longer it takes" );
		ImGui::InputDouble( "Frequency", &m_perlinFrequency );
		if( ImGui::IsItemHovered() )
			ImGui::SetTooltip( "Frequency of first octave (best 2-10) - detail level" );
		ImGui::InputDouble( "Persistence", &m_perlinPersistence );
		if( ImGui::IsItemHovered() )
			ImGui::SetTooltip( "Roughness of terrain - diminishing of amplitude between octaves" );
		ImGui::InputDouble( "Lacunarity", &m_perlinLacunarity );
		if( ImGui::IsItemHovered() )
			ImGui::SetTooltip( "Freq. multiplier between octaves - best results between 1.5 and 3.5." );
		ImGui::Separator();
		ImGui::Checkbox( "Billows module active", &m_billowsActive );
		ImGui::InputInt( "Octave count (4-30)", &m_billowsOctaveCount );
		ImGui::InputDouble( "Frequency", &m_billowsFrequency );
		ImGui::InputDouble( "Persistence", &m_billowsPersistence );
		ImGui::InputDouble( "Lacunarity", &m_billowsLacunarity );
		ImGui::Separator();
		ImGui::Checkbox( "Ridges module active", &m_ridgesActive );
		ImGui::InputInt( "Octave count (4-30)", &m_ridgesOctaveCount );
		ImGui::InputDouble( "Frequency", &m_ridgesFrequency );
		ImGui::InputDouble( "Lacunarity", &m_ridgesLacunarity );
		ImGui::Separator();
		ImGui::Checkbox( "Selector active", &m_selectorActive );
		if( ImGui::IsItemHovered() )
			ImGui::SetTooltip( "Selects from billows or ridges, criterion is set by perlin module." );
		ImGui::InputDouble( "Lower bound", &m_lowerBound );
		ImGui::InputDouble( "Upper bound", &m_upperBound );
		ImGui::InputDouble( "Falloff", &m_falloff );
		ImGui::Separator();
		ImGui::Text( "Generate:");
		ImGui::InputInt2( "Map width and height", &m_mapDimension[0] );
		if( ImGui::IsItemHovered() )
			ImGui::SetTooltip( "Powers of 2 and width double of height." );*/

// Cut out 2
/*if( 1 == clicked ) {
		std::cout << "Recalculating height map ...\n";
		noise::NoiseMap map{ 2048, 1024 };
		if( m_perlinActive ) {
			noise::Perlin perlin{ m_perlinFrequency, m_perlinLacunarity, m_perlinPersistence, m_perlinOctaveCount };
			noise::MapBuilderSphere{ &perlin, &map, 90.0, -90.0, 180.0, -180.0 };
		}
		{
			double min, max;
			map.getMinMaxValues( min, max );
			double offset{ std::abs( min ) };
			double bias{ 65535.0 / ( std::abs( max ) + std::abs( min ) ) };
			map.writePNG( "Resources/Textures/Ellipsoids/el_test.png", offset, bias );
			delete m_texture;
			m_texture = new Texture2D{ "Resources/Textures/Ellipsoids/el_test.png", 0, true };
			setDefaultSampler( m_texture->getName(), LINEAR_CLAMP );
		}
	}*/
