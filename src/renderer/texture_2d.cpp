
#include <base/logbook.h>
#include <renderer/texture_2d.h>
#include <sstream>
#include <algorithm>	// max()
#include <cmath>		// floor()
#include "stb/stb_image.h"

namespace orf_n {

texture_2d::texture_2d( const std::string &filename, const GLuint unit, bool generateMipmaps ) :
		texture{ GL_TEXTURE_2D, unit }, m_filename{ filename } {

	stbi_set_flip_vertically_on_load( true );
	uint8_t *data{ nullptr };
	data = stbi_load( filename.c_str(), &m_width, &m_height, &m_num_channels, 0 );
	if( data == nullptr ) {
		std::string s{ "2D texture " + filename + " could not be loaded." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error( s );
	}

	GLenum format;
	switch( m_num_channels ) {
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
					std::to_string( m_num_channels ) );
	}

	// number of mip levels
	if( generateMipmaps ) {
		int numLevels = 1 + (int)floor( log2( std::max( m_width, m_height ) ) );
		glTextureParameteri( m_texture_name, GL_TEXTURE_MAX_LEVEL, numLevels );
		glTextureStorage2D( m_texture_name, numLevels, m_format, m_width, m_height );
	} else
		glTextureStorage2D( m_texture_name, 1, m_format, m_width, m_height );

	glTextureSubImage2D( m_texture_name, 0,	// texture and mip level
			0, 0, m_width, m_height,		// offset and size
			format, GL_UNSIGNED_BYTE, data );

	bind_to_unit();

	std::string s{ "2D texture '" + filename + "' loaded, unit " + std::to_string( m_unit ) +
				   ", " + std::to_string( m_width ) + '*' + std::to_string( m_height ) + ", " +
				   std::to_string( m_num_channels ) + " channel(s)." };
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );

	if( generateMipmaps )
		create_mipmaps();

	// Set application memory free, ogl has our data after glTextureSubImage2D()
	if( nullptr != data )
		stbi_image_free( data );

}

texture_2d::texture_2d( const GLfloat *data, const omath::ivec2 extent, const GLuint unit, bool generateMipmaps ) :
	texture{ GL_TEXTURE_2D, unit }, m_width{extent.x}, m_height{extent.y}, m_format{GL_RGB32F} {
	GLenum format = GL_RGB;
	glTextureStorage2D( m_texture_name, 1, m_format, m_width, m_height );
	glTextureSubImage2D( m_texture_name, 0,	// texture and mip level
			0, 0, m_width, m_height,		// offset and size
			format, GL_FLOAT, data );
	bind_to_unit();
	std::string s{ "2D texture from data generated, unit " + std::to_string( m_unit ) +
					   ", " + std::to_string( m_width ) + '*' + std::to_string( m_height ) + ", " +
					   std::to_string( m_num_channels ) + " channel(s)." };
	if( generateMipmaps )
		create_mipmaps();
}

texture_2d::~texture_2d() {
	m_unit = 0;
	bind_to_unit();
	std::string t{ m_filename == "" ? "data texture" : m_filename };
	std::string s{ "2D texture '" + t + "'; texture name #" +
		std::to_string( m_texture_name ) + " destroyed." };
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
}

void texture_2d::create_mipmaps() {
	if( m_mipmaps_created )
		return;
	// mipmaps
	glTextureParameteri( m_texture_name, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTextureParameteri( m_texture_name, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTextureParameteri( m_texture_name, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_texture_name, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glGenerateTextureMipmap( m_texture_name );
	m_mipmaps_created = true;
}

void texture_2d::print_lod_info() const {
	GLint minLOD, maxLOD, baseLevel, maxLevel;
	glGetTextureParameteriv( m_texture_name, GL_TEXTURE_MIN_LOD, &minLOD );
	glGetTextureParameteriv( m_texture_name, GL_TEXTURE_MAX_LOD, &maxLOD );
	glGetTextureParameteriv( m_texture_name, GL_TEXTURE_BASE_LEVEL, &baseLevel );
	glGetTextureParameteriv( m_texture_name, GL_TEXTURE_MAX_LEVEL, &maxLevel );
	std::ostringstream s;
	s << "Texture " << m_texture_name << ": min LOD: " << minLOD <<
			", max LOD: " << maxLOD << ", base level: " << baseLevel << ", max level: " << maxLevel << ".";
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s.str() );
}

}	// namespace
