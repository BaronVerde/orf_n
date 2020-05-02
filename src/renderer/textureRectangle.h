
#pragma once

#include <renderer/texture.h>
#include <string>

namespace orf_n {

class TextureRectangle : public texture {
public:
	TextureRectangle( const std::string &filename,
					  const GLuint unit );

	TextureRectangle( const TextureRectangle &other ) = delete;

	TextureRectangle &operator=( const TextureRectangle &other ) = delete;

	virtual ~TextureRectangle();

	void createMipmaps();

private:
	std::string m_filename;

	GLsizei	m_size;

	GLenum	m_format;

	GLsizei	m_numChannels;

	bool m_mipmapsCreated = false;

};

}
