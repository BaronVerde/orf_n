
#include "shadow_map.h"
#include "scene/scene.h"
#include "applications/camera/camera.h"
#include "renderer/uniform.h"
#include "stb/stb_image.h"

shadow_map::shadow_map() : orf_n::renderable{"shadow_map"} {}

shadow_map::~shadow_map() {}

void shadow_map::setup() {
	m_scene->get_camera()->set_position_and_target( {0.0f, 0.0f, 3.0f }, {0.0f} );
	std::vector<std::shared_ptr<orf_n::module>> modules;
	modules.push_back( std::make_shared<orf_n::module>( GL_VERTEX_SHADER,
			"src/applications/shadow_map/shadow_mapping.vs" ) );
	modules.push_back( std::make_shared<orf_n::module>( GL_FRAGMENT_SHADER,
			"src/applications/shadow_map/shadow_mapping.fs" ) );
	m_shader_program = new orf_n::program( modules );
	modules.clear();
	modules.push_back( std::make_shared<orf_n::module>( GL_VERTEX_SHADER,
			"src/applications/shadow_map/shadow_mapping_depth.vs" ) );
	modules.push_back( std::make_shared<orf_n::module>( GL_FRAGMENT_SHADER,
			"src/applications/shadow_map/shadow_mapping_depth.fs" ) );
	m_simple_depth_shader = new orf_n::program( modules );
	modules.clear();
	modules.push_back( std::make_shared<orf_n::module>( GL_VERTEX_SHADER,
			"src/applications/shadow_map/debug_quad.vs" ) );
	modules.push_back( std::make_shared<orf_n::module>( GL_FRAGMENT_SHADER,
			"src/applications/shadow_map/debug_quad.fs" ) );
	m_debug_depth_quad = new orf_n::program( modules );
	// set up vertex data (and buffer(s)) and configure vertex attributes
	float planeVertices[] = {
			// positions            // normals         // texcoords
			 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
			-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
			 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
			 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 10.0f
	};
	// plane VAO
	glGenVertexArrays(1, &m_plane_vao);
	glGenBuffers(1, &m_plane_vbo);
	glBindVertexArray(m_plane_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_plane_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// load textures
	glGenTextures(1, &m_wood_texture);
	int width, height, nrComponents;
	unsigned char* data = stbi_load( "resources/textures/csm/wood.png", &width, &height, &nrComponents, 0);
	glBindTexture(GL_TEXTURE_2D, m_wood_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	// for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders.
	// Due to interpolation it takes texels from next repeat
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(data);
	// configure depth map FBO
	glGenFramebuffers(1, &m_depth_map_fbo);
	// create depth texture
	glGenTextures(1, &m_depth_map);
	glBindTexture(GL_TEXTURE_2D, m_depth_map);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_depth_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// shader configuration
	m_shader_program->use();
	orf_n::set_uniform( m_shader_program->getProgram(), "diffuseTexture", 0 );
	orf_n::set_uniform( m_shader_program->getProgram(), "shadowMap", 1 );
	m_debug_depth_quad->use();
	orf_n::set_uniform( m_debug_depth_quad->getProgram(), "m_depth_map", 0 );
	// lighting info
	m_light_pos = omath::vec3{ -2.0f, 4.0f, -1.0f };

	float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
			1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
	};
	glGenVertexArrays(1, &m_cube_vao);
	glGenBuffers(1, &m_cube_vbo);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(m_cube_vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// setup plane VAO
	glGenVertexArrays(1, &m_quad_vao);
	glGenBuffers(1, &m_quad_vbo);
	glBindVertexArray(m_quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void shadow_map::render() {
	// change light position over time
	//lightPos.x = sin(glfwGetTime()) * 3.0f;
	//lightPos.z = cos(glfwGetTime()) * 2.0f;
	//lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;
	// render
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// 1. render depth of scene to texture (from light's perspective)
	float near_plane = 1.0f, far_plane = 7.5f;
	const omath::mat4 lightProjection = omath::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	const omath::mat4 lightView = omath::lookAt( m_light_pos, omath::vec3(0.0f), omath::vec3(0.0, 1.0, 0.0));
	const omath::mat4 lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	m_simple_depth_shader->use();
	orf_n::set_uniform( m_simple_depth_shader->getProgram(), "lightSpaceMatrix", lightSpaceMatrix );
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, m_depth_map_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_wood_texture);
	render_scene(*m_simple_depth_shader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 2. render scene as normal using the generated depth/shadow map
	glViewport(0, 0, m_scene->get_window()->get_width(), m_scene->get_window()->get_height() );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	m_shader_program->use();
	omath::mat4 projection = omath::perspective( omath::radians( m_scene->get_camera()->get_zoom() ),
			(float)m_scene->get_window()->get_width() / (float)m_scene->get_window()->get_height(), 0.1f, 100.0f
	);
	const omath::mat4 view = m_scene->get_camera()->get_view_matrix();
	orf_n::set_uniform( m_shader_program->getProgram(), "projection", projection );
	orf_n::set_uniform( m_shader_program->getProgram(), "view", view);
	// set light uniforms
	orf_n::set_uniform(
			m_shader_program->getProgram(), "viewPos", omath::vec3{ m_scene->get_camera()->get_position() }
	);
	orf_n::set_uniform( m_shader_program->getProgram(), "lightPos", m_light_pos);
	orf_n::set_uniform( m_shader_program->getProgram(), "lightSpaceMatrix", lightSpaceMatrix);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_wood_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_depth_map);
	render_scene(*m_shader_program);
	// render Depth map to quad for visual debugging
	glViewport( 0, 0, 256, 256 );
	m_debug_depth_quad->use();
	orf_n::set_uniform( m_debug_depth_quad->getProgram(), "near_plane", near_plane);
	orf_n::set_uniform( m_debug_depth_quad->getProgram(), "far_plane", far_plane);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_depth_map);
	render_quad();
}

void shadow_map::cleanup() {
	delete m_debug_depth_quad;
	delete m_simple_depth_shader;
	delete m_shader_program;
}

// renders the 3D scene
void shadow_map::render_scene( const orf_n::program& shader ) {
	// floor
	omath::mat4 model = omath::mat4(1.0f);
	orf_n::set_uniform( shader.getProgram(), "model", model );
	glBindVertexArray(m_plane_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = omath::mat4(1.0f);
	model = omath::translate(model, omath::vec3(0.0f, 1.5f, 0.0));
	model = omath::scale(model, omath::vec3(0.5f));
	orf_n::set_uniform( shader.getProgram(), "model", model);
	render_cube();
	model = omath::mat4(1.0f);
	model = omath::translate(model, omath::vec3(2.0f, 0.0f, 1.0));
	model = omath::scale(model, omath::vec3(0.5f));
	orf_n::set_uniform( shader.getProgram(), "model", model);
	render_cube();
	model = omath::mat4(1.0f);
	model = omath::translate(model, omath::vec3(-1.0f, 0.0f, 2.0));
	model = omath::rotate(model, omath::radians(60.0f), omath::normalize(omath::vec3(1.0, 0.0, 1.0)));
	model = omath::scale(model, omath::vec3(0.25));
	orf_n::set_uniform( shader.getProgram(), "model", model);
	render_cube();
}

void shadow_map::render_cube() {
	glBindVertexArray(m_cube_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void shadow_map::render_quad() {
	glBindVertexArray(m_quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
