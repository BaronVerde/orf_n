
#include <renderer/uniform.h>
#include <scene/scene.h>
#include "SkyBox.h"
#include "renderer/VertexArray3D.h"

using namespace orf_n;

SkyBox::SkyBox( const std::vector<std::string> &files ) :
		renderable{ "SkyBox" } {
	if( 6 != files.size() ) {
		std::vector<std::string> faces{
			/*"resources/cubemaps/starfield_rt.tga",
			"resources/cubemaps/starfield_lf.tga",
			"resources/cubemaps/starfield_up.tga",
			"resources/cubemaps/starfield_dn.tga",
			"resources/cubemaps/starfield_ft.tga",
			"resources/cubemaps/starfield_bk.tga"*/
			"resources/cubemaps/eso_pos_x.png",
			"resources/cubemaps/eso_neg_x.png",
			"resources/cubemaps/eso_pos_y.png",
			"resources/cubemaps/eso_neg_y.png",
			"resources/cubemaps/eso_pos_z.png",
			"resources/cubemaps/eso_neg_z.png"
			/*"resources/cubemaps/nasa_pos_x.png",
			"resources/cubemaps/nasa_neg_x.png",
			"resources/cubemaps/nasa_pos_y.png",
			"resources/cubemaps/nasa_neg_y.png",
			"resources/cubemaps/nasa_pos_z.png",
			"resources/cubemaps/nasa_neg_z.png"*/
		};
		m_texture = std::make_unique<TextureCube>( faces );
	} else {
		m_texture = std::make_unique<TextureCube>( files );
	}

}

void SkyBox::setup() {
	// Create the shaders
	std::vector<std::shared_ptr<module>> modules;
	modules.push_back( std::make_shared<module>( GL_VERTEX_SHADER,
			"src/applications/SkyBox/Skybox.vert.glsl" ) );
	modules.push_back( std::make_shared<module>( GL_FRAGMENT_SHADER,
			"src/applications/SkyBox/Skybox.frag.glsl" ) );
	m_program = std::make_unique<program>( modules );

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
	glDepthFunc( GL_LESS );
    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );

	m_program->use();
	m_skyboxArray->bind();
	m_texture->bind();
	// Set view and projection matrix
	set_uniform( m_program->getProgram(), "projectionView",
			m_scene->get_camera()->get_untranslated_view_perspective_matrix() );
	glDrawArrays( GL_TRIANGLES, 0, 36 );

	glDepthFunc( GL_LESS );
}

void SkyBox::cleanup() {}

SkyBox::~SkyBox() {}
