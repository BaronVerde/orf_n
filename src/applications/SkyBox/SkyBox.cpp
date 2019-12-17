
#include "SkyBox.h"
#include "renderer/Uniform.h"
#include "renderer/VertexArray3D.h"
#include "scene/Scene.h"

using namespace orf_n;

SkyBox::SkyBox( const std::vector<std::string> &files ) :
		Renderable{ "SkyBox" } {
	if( 6 != files.size() ) {
		std::vector<std::string> faces{
			/*"resources/Cubemaps/starfield_rt.tga",
			"resources/Cubemaps/starfield_lf.tga",
			"resources/Cubemaps/starfield_up.tga",
			"resources/Cubemaps/starfield_dn.tga",
			"resources/Cubemaps/starfield_ft.tga",
			"resources/Cubemaps/starfield_bk.tga"*/
			"resources/Cubemaps/eso_pos_x.png",
			"resources/Cubemaps/eso_neg_x.png",
			"resources/Cubemaps/eso_pos_y.png",
			"resources/Cubemaps/eso_neg_y.png",
			"resources/Cubemaps/eso_pos_z.png",
			"resources/Cubemaps/eso_neg_z.png"
			/*"resources/Cubemaps/nasa_pos_x.png",
			"resources/Cubemaps/nasa_neg_x.png",
			"resources/Cubemaps/nasa_pos_y.png",
			"resources/Cubemaps/nasa_neg_y.png",
			"resources/Cubemaps/nasa_pos_z.png",
			"resources/Cubemaps/nasa_neg_z.png"*/
		};
		m_texture = std::make_unique<TextureCube>( faces );
	} else {
		m_texture = std::make_unique<TextureCube>( files );
	}

}

void SkyBox::setup() {
	// Create the shaders
	std::vector<std::shared_ptr<Module>> modules;
	modules.push_back( std::make_shared<Module>( GL_VERTEX_SHADER,
			"src/applications/SkyBox/Skybox.vert.glsl" ) );
	modules.push_back( std::make_shared<Module>( GL_FRAGMENT_SHADER,
			"src/applications/SkyBox/Skybox.frag.glsl" ) );
	m_program = std::make_unique<Program>( modules );

	std::vector<omath::vec3> skyboxVertices{
		// positions
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f,  1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{  1.0f,  1.0f, -1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f,  1.0f }
	};
	// Binding index 9
	m_skyboxArray = std::make_unique<orf_n::VertexArray3D<omath::vec3>>( skyboxVertices, 0 );

}

void SkyBox::render() {
	// draw skybox at last !
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );

	m_program->use();
	m_skyboxArray->bind();
	m_texture->bind();
	// Set view and projection matrix
	setUniform( m_program->getProgram(), "projectionView",
			m_scene->getCamera()->getUntranslatedViewPerspectiveMatrix() );
	glDrawArrays( GL_TRIANGLES, 0, 36 );

	glDepthFunc( GL_LESS );
}

void SkyBox::cleanup() {}

SkyBox::~SkyBox() {}
