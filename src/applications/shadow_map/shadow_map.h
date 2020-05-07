
#pragma once

#include "scene/renderable.h"
#include "renderer/program.h"
#include "renderer/texture_2d.h"
#include "omath/vec3.h"

class shadow_map : public orf_n::renderable {
public:
	shadow_map();

	virtual ~shadow_map();

	virtual void setup() override;

	virtual void render() override;

	virtual void cleanup() override;

private:

	const unsigned int SHADOW_WIDTH = 4096;
	const unsigned int SHADOW_HEIGHT = 4096;

	orf_n::program* m_shader_program{nullptr};

	orf_n::program* m_simple_depth_shader{nullptr};

	orf_n::program* m_debug_depth_quad{nullptr};

	GLuint m_plane_vao, m_plane_vbo;

	GLuint m_cube_vao, m_cube_vbo;

	GLuint m_quad_vao, m_quad_vbo;

	GLuint m_wood_texture;

	GLuint m_depth_map_fbo, m_depth_map;

	omath::vec3 m_light_pos{ -2.0f, 4.0f, -1.0f };

	void render_scene( const orf_n::program& shader );

	void render_cube();

	void render_quad();

};
