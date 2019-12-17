
/**
 * A class to handle 2D textures.
 * 8 bit RGB and 8 bit RGBA are supported for now. Also default sampler values.
 * @todo: mutisampling, separate samplers
 */

#pragma once

#include "omath/vec2.h"
#include "Texture.h"
#include <string>

namespace orf_n {

class Texture2D : public Texture {
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
	Texture2D( const std::string &filename, const GLuint unit, bool generateMipmaps = false );

	/**
	 * Create a 3 channel float (GL_RGB32F) texture from given data.
	 */
	Texture2D( const GLfloat *data, const omath::ivec2 extent, const GLuint unit, bool generateMipmaps = false );

	Texture2D( const Texture2D &other ) = delete;

	Texture2D &operator=( const Texture2D &other ) = delete;

	virtual ~Texture2D();

	void createMipmaps();

	/**
	 * Prints number of levels to logbook.
	 */
	void printLODInfo() const;

private:

	std::string m_filename;

	GLsizei	m_width;

	GLsizei	m_height;

	GLenum	m_format;

	GLsizei	m_numChannels;

	bool m_mipmapsCreated = false;

};

}	// namespace
