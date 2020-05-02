
#include <base/logbook.h>
#include <renderer/textureRectangle.h>
#include <cmath>	// floor()
#include "stb/stb_image.h"

namespace orf_n {

TextureRectangle::TextureRectangle( const std::string &filename,
									const GLuint unit )  :
		texture{ GL_TEXTURE_RECTANGLE, unit }, m_filename{ filename } {

	stbi_set_flip_vertically_on_load( true );
	uint8_t *data{ nullptr };
	int w, h;
	data = stbi_load( filename.c_str(), &w, &h, &m_numChannels, 0 );
	if( data == nullptr ) {
		std::string s{ "Rectangle texture " + filename + " could not be loaded." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	if( w != h ) {
		std::string s{ "Texture " + filename + " is not a rectangle. Failed to load." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		if( nullptr != data )
			stbi_image_free( data );
		throw std::runtime_error( s );
	}

	m_size = w;
	GLenum format;
	switch( m_numChannels ) {
		case 1:
			m_format = GL_R8;
			format = GL_RED;
			break;
		case 3:
			m_format = GL_RGB8;
			format = GL_RGB;
			break;
			// RGBA 8 bit
		case 4:
			m_format = GL_RGBA8;
			format = GL_RGBA;
			break;
		default:
			logbook::log_msg( logbook::RENDERER, logbook::WARNING,
					"2D texture " + filename + " unsupported nr of channels for chosen format: " +
					std::to_string( m_numChannels ) );
	}

	glTextureStorage2D( m_texture_name, 1, m_format, m_size, m_size );

	glTextureSubImage2D( m_texture_name, 0,	// texture and mip level
			0, 0, m_size, m_size,			// offset and size
			format, GL_UNSIGNED_BYTE, data );

	bind_to_unit();

	std::string s{ "Rectangle texture '" + filename + "' loaded, unit " + std::to_string( m_unit ) +
				   ", size " + std::to_string( m_size ) + ", " +
				   std::to_string( m_numChannels ) + " channel(s)." };
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );

	// Set application memory free, ogl has our data after glTextureSubImage2D()
	if( nullptr != data )
		stbi_image_free( data );

}

TextureRectangle::~TextureRectangle() {
	std::string s{ "Rectangle texture '" + m_filename + "' destroyed." };
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
}

void TextureRectangle::createMipmaps() {
	if( m_mipmapsCreated )
		return;
	// mipmaps
	glTextureParameteri( m_texture_name, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTextureParameteri( m_texture_name, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTextureParameteri( m_texture_name, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_texture_name, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glGenerateTextureMipmap( m_texture_name );
	m_mipmapsCreated = true;
}

} /* namespace orf_n */
