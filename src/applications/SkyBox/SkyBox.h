
/**
 * Draws a skybox from an vector of 6 files.
 * The files must come in sequence right, left, up, down, front, back image.
 * Default values in cpp file.
 * The skybox must be the first to be drawn to be in the background.
 * Sequence of faces:
 * GL_TEXTURE_CUBE_MAP_POSITIVE_X 	Right	0
 * GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	Left	1
 * GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	Top		2
 * GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	Bottom	3
 * GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	Back	4
 * GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	Front	5
 */

#pragma once

#include <renderer/program.h>
#include <renderer/textureCube.h>
#include <scene/renderable.h>
#include "omath/vec3.h"
#include "renderer/VertexArray3D.h"
#include <memory>

class SkyBox : public orf_n::renderable {
public:
	SkyBox( const std::vector<std::string> &files = {} );

	virtual ~SkyBox();

	void setup() override final;

	void render() override final;

	void cleanup() override final;

private:
	std::unique_ptr<orf_n::TextureCube> m_texture{ nullptr };

	std::unique_ptr<orf_n::program> m_program{ nullptr };

	std::unique_ptr<orf_n::VertexArray3D<omath::vec3>> m_skyboxArray{ nullptr };

};
