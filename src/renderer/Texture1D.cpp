
#include <base/logbook.h>
#include "Texture1D.h"
#include "stb/stb_image.h"

namespace orf_n {

Texture1D::Texture1D( const std::string &filename, const GLuint unit ) :
			Texture{ GL_TEXTURE_1D, unit }, m_filename{ filename } {
	uint8_t *data{ nullptr };
	int h{ 0 };
	data = stbi_load( filename.c_str(), &m_width, &h, &m_numChannels, 0 );
	if( data == nullptr ) {
		std::string s{ "1D texture " + filename + " could not be loaded." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error( s );
	}

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
					"1D texture " + filename + " unsupported nr of channels for chosen format: " +
					std::to_string( m_numChannels ) );
	}
	glTextureStorage2D( m_textureName, 1, m_format, m_width, h );
	glTextureSubImage2D( m_textureName, 0,	// texture and mip level
					0, 0, m_width, h,		// offset and size
					format, GL_UNSIGNED_BYTE, data );

	bindToUnit();

	// Set application memory free, ogl has our data after glTextureSubImage2D()
	if( nullptr != data )
		stbi_image_free( data );

	std::string s{ "1D texture '" + m_filename + "'; texture name #" +
		std::to_string( m_textureName ) + "loaded." };
	logbook::log_msg( logbook::SHADER, logbook::INFO, s );

}

Texture1D::~Texture1D() {
	std::string s{ "1D texture '" + m_filename + "'; texture name #" +
		std::to_string( m_textureName ) + "destroyed." };
	logbook::log_msg( logbook::SHADER, logbook::INFO, s );
}

} /* namespace orf_n */
