
#pragma once

#include <renderer/texture.h>
#include <string>

namespace orf_n {

/**
 * A 1D texture. e.g. for color ramps.
 */
class Texture1D : public texture {
public:
	/**
	 * Create a new 1D texture
	 * @param filename Texture to load.
	 * @param unit Shader texture unit for binding.
	 */
	Texture1D( const std::string &filename,
			   const GLuint unit );

	Texture1D( const Texture1D &other ) = delete;

	Texture1D &operator=( const Texture1D &other ) = delete;

	virtual ~Texture1D();

private:
	GLsizei	m_width;

	GLenum	m_format;

	GLsizei	m_numChannels;

	std::string m_filename;

};

}
