
#include <applications/SkyBox/SkyBox.h>
#include <renderer/Uniform.h>
#include <renderer/VertexArray3D.h>
#include <scene/Scene.h>

using namespace orf_n;

SkyBox::SkyBox( const std::vector<std::string> &files ) :
		Renderable{ "SkyBox" } {
	if( 6 != files.size() ) {
		std::vector<std::string> faces{
			/*"Resources/Cubemaps/starfield_rt.tga",
			"Resources/Cubemaps/starfield_lf.tga",
			"Resources/Cubemaps/starfield_up.tga",
			"Resources/Cubemaps/starfield_dn.tga",
			"Resources/Cubemaps/starfield_ft.tga",
			"Resources/Cubemaps/starfield_bk.tga"*/
			"Resources/Cubemaps/eso_pos_x.png",
			"Resources/Cubemaps/eso_neg_x.png",
			"Resources/Cubemaps/eso_pos_y.png",
			"Resources/Cubemaps/eso_neg_y.png",
			"Resources/Cubemaps/eso_pos_z.png",
			"Resources/Cubemaps/eso_neg_z.png"
			/*"Resources/Cubemaps/nasa_pos_x.png",
			"Resources/Cubemaps/nasa_neg_x.png",
			"Resources/Cubemaps/nasa_pos_y.png",
			"Resources/Cubemaps/nasa_neg_y.png",
			"Resources/Cubemaps/nasa_pos_z.png",
			"Resources/Cubemaps/nasa_neg_z.png"*/
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
			"Source/Applications/SkyBox/Skybox.vert.glsl" ) );
	modules.push_back( std::make_shared<Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/SkyBox/Skybox.frag.glsl" ) );
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
