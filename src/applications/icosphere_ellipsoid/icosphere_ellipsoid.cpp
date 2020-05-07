
#include <renderer/uniform.h>
#include "icosphere_ellipsoid.h"
#include "base/globals.h"
#include "base/logbook.h"
#include "renderer/sampler.h"
#include "scene/scene.h"
#include "glad/glad.h"
#include "omath/mat4.h"
#include "renderer/Color.h"
#include <iostream>
#include <fstream>
//#include "Geometry/ViewFrustum.h"

using namespace orf_n;

extern bool globals::show_app_ui;

icosphere_ellipsoid::icosphere_ellipsoid(	const omath::vec3 &axes,
										const uint32_t numSubDivs ) :
				renderable{ "icosphere_ellipsoid" },
				m_axes{ axes },
				m_numSubDivs{ numSubDivs } {}

icosphere_ellipsoid::~icosphere_ellipsoid() {
	logbook::log_msg( logbook::RENDERER, logbook::INFO,
			"IcosphereEllipsoid cleaned up and deleted." );
}

void icosphere_ellipsoid::setup() {
	// Create the shaders
	std::vector<std::shared_ptr<module>> modules;
	modules.push_back( std::make_shared<module>( GL_VERTEX_SHADER,
			"src/applications/icosphere_ellipsoid/icosphere_ellipsoid.vert.glsl" ) );
	modules.push_back( std::make_shared<module>( GL_FRAGMENT_SHADER,
			"src/applications/icosphere_ellipsoid/icosphere_ellipsoid.frag.glsl" ) );
	m_shader = new program( modules );

	m_texture = new texture_2d{ "resources/Textures/Ellipsoids/EarthDay_4k.jpg", 0, true };
	//m_texture = new Texture2D{ "Resources/Textures/Ellipsoids/sphere1.png", 0, true };
	set_default_sampler( m_texture->get_name(), LINEAR_CLAMP );

	m_ico = new icosphere{ m_axes, m_numSubDivs };
	m_vertexArray = new VertexArray3D<omath::dvec3>{ m_ico->get_vertices(), 0, true };
	m_indexBuffer = new IndexBuffer{ m_ico->get_indices() };

	m_scene->get_camera()->set_position_and_target( { 0.0, 0.0, -15000000.0  }, { 0.0, 0.0, 0.0 } );
	//m_scene->get_camera()->setUp( { 0.0f, 0.0f, 1.0f } );
	m_scene->get_camera()->set_near_plane( 100.0f );
	m_scene->get_camera()->set_far_plane( 100000000.0f );
	m_scene->get_camera()->set_movement_speed( 300.0f );
	m_scene->get_camera()->calculate_fov();
	m_shader->use();
	set_uniform( m_shader->getProgram(), "u_oneOverRadiiSquared",
			static_cast<omath::vec3>( m_ico->getOneOverRadiiSquared() ) );

	// read bounding boxes and fill m_relativeBoxes and m_geodeticCoords
	std::vector<omath::dvec3> boxOutlines;
	boxOutlines.resize( 289 * 4 );
	std::vector<GLuint> indices;
	for( int i{0}; i < 289; ++i ) {
		readBB( "resources/Textures/Terrain/Area_30_00/tile_2048_" + std::to_string(i+1) + ".bb" );
		boxOutlines[i*4] = m_ico->to_cartesian( m_geodeticCoords[i] );
		const double delta{ omath::radians( 2048 * m_cellsize ) };
		boxOutlines[i*4+1] = m_ico->to_cartesian(
				geodetic{ m_geodeticCoords[i].get_longitude() + delta, m_geodeticCoords[i].get_latitude() }
		);
		boxOutlines[i*4+2] = m_ico->to_cartesian(
				geodetic{ m_geodeticCoords[i].get_longitude() + delta, m_geodeticCoords[i].get_latitude() + delta }
		);
		boxOutlines[i*4+3] = m_ico->to_cartesian(
				geodetic{ m_geodeticCoords[i].get_longitude(), m_geodeticCoords[i].get_latitude() + delta }
		);
		indices.push_back( i*4 );
		indices.push_back( i*4+1 );
		indices.push_back( i*4+2 );
		indices.push_back( i*4+3 );
		indices.push_back( 999 );
	}
	//std::cout << boxOutlines.size() << "; " << boxOutlines[(289*4)-1] << std::endl;
	//std::cout << indices.size() << std::endl;

	m_tileBorderArray = new orf_n::VertexArray3D{ boxOutlines, 0, true };
	m_tileBorderIndices = new orf_n::IndexBuffer{ indices };
	glPrimitiveRestartIndex( 999 );

	logbook::log_msg( logbook::RENDERER, logbook::INFO, "IcosphereEllipsoid created and set up." );
}

void icosphere_ellipsoid::render() {
	orf_n::camera *cam{ m_scene->get_camera() };	// shortcut
	float nearPlane{ cam->get_near_plane() };
	float farPlane{ cam->get_far_plane() };
	float movementSpeed{ cam->get_movement_speed() };
	if( globals::show_app_ui ) {
		ImGui::Begin( "Ellipsoid Params" );
		//ImGui::Checkbox( "Origin in frustum: ", &m_visible );
		ImGui::SliderFloat( "Near plane", &nearPlane, 0.01f, 100.0f );
		ImGui::SliderFloat( "Far plane", &farPlane, 10.0f, 100000000.0f );
		ImGui::SliderFloat( "Movement speed", &movementSpeed, 10.0f, 100000.0f );
		/*ImGui::Checkbox( "Height displacement", &m_showDisplacement );
		// Cut out 1
		ImGui::Text( "Calculate normals:" );
		ImGui::RadioButton( "centric", &m_normalType, 0 ); ImGui::SameLine();
		ImGui::RadioButton( "detic", &m_normalType, 1 ); ImGui::SameLine();
		ImGui::RadioButton( "combined", &m_normalType, 2 );
		if( ImGui::Button( "Calculate Height Map" ) )
			clicked++;*/
		ImGui::End();
	}
	if( nearPlane != cam->get_near_plane() )
		cam->set_near_plane( nearPlane );
	if( farPlane != cam->get_far_plane() )
		cam->set_far_plane( farPlane );
	if( movementSpeed != cam->get_movement_speed() )
		cam->set_movement_speed( movementSpeed );

	m_shader->use();
	m_visible = true;
	/*m_visible = ( OUTSIDE != cam->getViewFrustum().isSphereInFrustum(
			omath::vec3( 0.0f, 0.0f, 0.0f ), (float)m_ico->getRadii().x ) );*/
	if( m_visible ) {
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_CULL_FACE );
		// glFrontFace( GL_CCW ); glCullFace( GL_BACK );
		m_vertexArray->bind();
		m_indexBuffer->bind();
		const omath::dmat4 view{ omath::lookAt(
			cam->get_position(),
			cam->get_position() + omath::dvec3{ cam->get_front() },
			omath::dvec3{ cam->get_up() } )
		};
		const omath::dmat4 mv{ view * m_modelMatrix };
		// this matrix can be used for all objects in same coord system
		const omath::mat4 mvRTE{
			omath::vec4{ mv[0] },
			omath::vec4{ mv[1] },
			omath::vec4{ mv[2] },
			omath::vec4{ 0.0f, 0.0f, 0.0f, static_cast<float>( mv[3][3] ) }
		};
		setModelViewProjectionMatrixRTE( cam->get_perspective_matrix() * mvRTE );
		// setCameraPosition() with double converts to two floats and sets high and low pos
		setCameraPosition( cam->get_position() );
		orf_n::set_uniform( m_shader->getProgram(), "u_drawColor", orf_n::color::green * 0.5f );
		glPolygonMode( GL_FRONT_AND_BACK, cam->get_wireframe_mode() ? GL_LINE : GL_FILL );
		glDrawElements( GL_TRIANGLES, m_indexBuffer->getNumber(), GL_UNSIGNED_INT, NULL );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		// Debug
		m_tileBorderArray->bind();
		m_tileBorderIndices->bind();
		orf_n::set_uniform( m_shader->getProgram(), "u_drawColor", orf_n::color::blue );
		glEnable( GL_PRIMITIVE_RESTART );
		glDrawElements( GL_LINE_LOOP, m_tileBorderIndices->getNumber(), GL_UNSIGNED_INT, (void *)0 );
		glDisable( GL_PRIMITIVE_RESTART );

	}	// visible

}

void icosphere_ellipsoid::cleanup() {
	delete m_tileBorderIndices;
	delete m_tileBorderArray;
	delete m_texture;
	delete m_vertexArray;
	delete m_boxArray;
	delete m_shader;
	delete m_indexBuffer;
	delete m_ico;
}

void icosphere_ellipsoid::readBB( const std::string &filename ) {
	std::ifstream bbfile{ filename, std::ios::in };
	if( bbfile.is_open() ) {
		omath::dvec3 min, max;
		bbfile >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
		m_relativeBoxes.push_back( orf_n::aabb{ min, max } );
		double lon, lat;
		bbfile >> lon >> lat >> m_cellsize;
		m_geodeticCoords.push_back( orf_n::geodetic{ omath::radians(lon), omath::radians(lat) } );
		if( !(m_cellsize >= 0.0) )
			orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::ERROR,
					"Terrain tile bounding boxes file: invalid cell size" );
		bbfile.close();
	} else
		orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::ERROR,
				"Terrain tile bounding boxes file '" + filename + "' could not be opened" );
}

// Cut out
/*ImGui::Separator();
		ImGui::Checkbox( "Perlin module active", &m_perlinActive );
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
