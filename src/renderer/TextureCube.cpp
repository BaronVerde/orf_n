
#include <base/logbook.h>
#include "TextureCube.h"
#include <sstream>
#include "stb/stb_image.h"

namespace orf_n {

TextureCube::TextureCube( const std::vector<std::string> &files ) {
	glCreateTextures( GL_TEXTURE_CUBE_MAP, 1, &m_textureName );

	// Check size of 1st face
	int width, height, numChannels;
	//stbi_set_flip_vertically_on_load( true );
	stbi_uc *data = stbi_load( files[0].c_str(), &width, &height, &numChannels, 0 );
	if( NULL == data ) {
		std::string s{ "Cubemap texture '" + files[0] + "' failed to load." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error( s );
	} else
		// Only a test.
		stbi_image_free( data );
	GLuint internalFormat, format;
	switch( numChannels ) {
		case 1 : internalFormat = GL_R8; format = GL_RED; break;
		case 3 : internalFormat = GL_RGB8; format = GL_RGB; break;
		case 4 : internalFormat = GL_RGBA8; format = GL_RGBA; break;
		default :
			std::string s{ "Unsupported cube map texture format. '" + files[0] +
				"' failed to load. Check number of channels (1/3/4)." };
			logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
			throw std::runtime_error( s );
			break;
	}

	glTextureStorage2D( m_textureName, 1, internalFormat, width, height );
	// One mip level only
	for( unsigned int face{ 0 }; face < 6; ++face ) {
		data = stbi_load( files[face].c_str(), &width, &height, &numChannels, 0 );
		if( NULL != data ) {
			glTextureSubImage3D( m_textureName, 0, 0, 0,// target, level , xOffset, yOffset
					face, width, height, 1, format,		// cube map face, width, height, 1 face a time, format
					GL_UNSIGNED_BYTE, data );			// datatype, data pointer
			stbi_image_free( data );
			data = NULL;
		} else {
			std::string s{ "Cubemap texture '" + files[face] + "' failed to load." };
			logbook::log_msg( logbook::RENDERER, logbook::WARNING, s );
		}
	}
	glTextureParameteri( m_textureName, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTextureParameteri( m_textureName, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTextureParameteri( m_textureName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_textureName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_textureName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

	std::ostringstream s;
	s << "Cubemap '" << files[0] << "' (" << width << '*' << height <<
		") " << numChannels << " channels and following 5 files loaded.";
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s.str() );

}

void TextureCube::bind() const {
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_textureName );
}

TextureCube::~TextureCube() {
	glDeleteTextures( 1, &m_textureName );
	std::string s{ "Cubemap texture #" + std::to_string( m_textureName ) + " destroyed." };
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
}

}
