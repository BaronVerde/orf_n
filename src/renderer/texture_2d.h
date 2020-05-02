
/**
 * A class to handle 2D textures.
 * 8 bit RGB and 8 bit RGBA are supported for now. Also default sampler values.
 * @todo: mutisampling, separate samplers
 */

#pragma once

#include <renderer/texture.h>
#include "omath/vec2.h"
#include <string>

namespace orf_n {

class texture_2d : public texture {
public:
	/**
	 * Ctor to load a 2d image file, png, jpg or the like.
	 * Imports an image file as a 2d texture with n channels with
	 * 8bit unsigned char data. Creates mipmaps and sets default
	 * sampler LINEAR/LINEAR and clamp to border.
	 *
	 * @param filename Texture to load.
	 * @param unit Shader texture unit for binding.
	 */
	texture_2d( const std::string &filename, const GLuint unit, bool generateMipmaps = false );

	/**
	 * Create a 3 channel float (GL_RGB32F) texture from given data.
	 */
	texture_2d( const GLfloat *data, const omath::ivec2 extent, const GLuint unit, bool generateMipmaps = false );

	texture_2d( const texture_2d &other ) = delete;

	texture_2d &operator=( const texture_2d &other ) = delete;

	virtual ~texture_2d();

	void create_mipmaps();

	/**
	 * Prints number of levels to logbook.
	 */
	void print_lod_info() const;

private:

	std::string m_filename;

	GLsizei	m_width;

	GLsizei	m_height;

	GLenum	m_format;

	GLsizei	m_num_channels;

	bool m_mipmaps_created = false;

};

}	// namespace
