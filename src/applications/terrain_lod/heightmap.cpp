
#include <applications/terrain_lod/heightmap.h>
#include <base/logbook.h>
#include <renderer/sampler.h>
#include <iostream>
#include <sstream>
#include "stb/stb_image.h"

using namespace orf_n;

namespace terrain {

/**
 * Create anew height map texture from file.
 * @param fielname Filename of the texture file. Supported format single channel 16bit png.
 * Use asc2png.cpp to create height map textures from srtm data.
 * For the shader: heightmap texture is bound to texture unit 0, high positions texture to unit 1,
 * low positions to unit 2.
 */
heightmap::heightmap( const std::string &filename, const bitDepth_t depth ) :
				m_filename{ filename }, m_bitDepth{ depth } {

	uint16_t *heightValues16{nullptr};
	uint8_t *heightValues8{nullptr};
	//stbi_set_flip_vertically_on_load( true );
	int w, h, numChannels;
	if( B16 == depth )
		// load the data, single channel 16
		heightValues16 = stbi_load_16( m_filename.c_str(), &w, &h, &numChannels, 1 );
	if( B8 == depth ) {
		// load the data, single channel 8
		heightValues8 = stbi_load( m_filename.c_str(), &w, &h, &numChannels, 1 );
	}
	if( nullptr == heightValues16 && nullptr == heightValues8 ) {
		std::string s{ "Error loading heightmap image file '" + m_filename + "'." };
		logbook::log_msg( logbook::TERRAIN, logbook::ERROR, s );
		// @todo throw std::runtime_error( s );
	}
	if( numChannels != 1 )
		logbook::log_msg( logbook::TERRAIN, logbook::WARNING,
				"Unknown heightmap format in '" + m_filename + "'. Not a monochrome image ?" );

	m_extent = omath::uvec2( static_cast<unsigned int>(w), static_cast<unsigned int>(h) );
	int numPixels{ m_extent.x * m_extent.y };
	// Unclamped values are allways stored as 16 bit integers
	m_heightValuesNormalized = new float[numPixels];
	// find min/max values
	unsigned int i{ 0 };
	m_minMaxHeightValues = omath::uvec2( 65535, 0 );
	for( int x{ 0 }; x < m_extent.x; ++x ) {
		for( int y{ 0 }; y < m_extent.y; ++y ) {
			uint16_t t;
			if( B8 == depth )
				t = heightValues8[x+m_extent.x*y];
			else
				t = heightValues16[x+m_extent.x*y];
			if( t > m_minMaxHeightValues.y )
				m_minMaxHeightValues.y = t;
			if( t < m_minMaxHeightValues.x )
				m_minMaxHeightValues.x = t;
			if( B8 == depth )
				m_heightValuesNormalized[i] = static_cast<float>(t) / 255.0f;
			else
				m_heightValuesNormalized[i] = static_cast<float>(t) / 65535.0f;
			i++;
		}
	}

	// There's only float data 0..1 from now on
	glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
	// no mip levels
	glTextureStorage2D( m_texture, 1, GL_R32F, m_extent.x, m_extent.y );
	glTextureSubImage2D( m_texture, 0,		// texture and mip level
			0, 0, m_extent.x, m_extent.y,	// offset and size
			GL_RED, GL_FLOAT, m_heightValuesNormalized );
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, m_texture );
	// set the default sampler for the heightmap texture
	set_default_sampler( m_texture, LINEAR_CLAMP );

	// release mem
	if( nullptr != heightValues8 )
		stbi_image_free( heightValues8 );
	if( nullptr != heightValues16 )
		stbi_image_free( heightValues16 );

	// @todo: query texture size !
	float totalSizeInKB{
		static_cast<float>( sizeof( *this ) + numPixels * sizeof( float ) ) / 1024.0f
	};
	std::ostringstream s;
	s << "Heightmap '" << m_filename << "', texture unit " << HEIGHTMAP_TEXTURE_UNIT <<
			", " << m_extent.x << '*' << m_extent.y << ", " << numChannels <<
			" channel(s) loaded. Size in memory : " << totalSizeInKB << "kB.";
	logbook::log_msg( logbook::TERRAIN, logbook::INFO, s.str() );
}

const omath::vec2 &heightmap::getMinMaxHeight() const {
	return m_minMaxHeightValues;
}

const GLuint &heightmap::getTexture() const {
	return m_texture;
}

const omath::ivec2 &heightmap::getExtent() const {
	return m_extent;
}

float heightmap::getHeightAt( const int x, const int y ) const {
	// @todo temp ! divided by 100
	return m_heightValuesNormalized[x + y * m_extent.x] * 655.35f;
}

omath::vec2 heightmap::getMinMaxHeightArea( const int x, const int z, const int w, const int h ) const {
	omath::vec2 values{ std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
	for( int i = x; i < x + w; ++i )
	    for( int j = z; j < z + h; ++j ) {
	    	float newVal = getHeightAt( i, j );
	        if( newVal < values.x )
	            values.x = newVal;
	        if( newVal > values.y )
	        	values.y = newVal;
	    }
	return omath::vec2{ values };
}

const heightmap::bitDepth_t &heightmap::getDepth() const {
	return m_bitDepth;
}

heightmap::~heightmap() {
	unbind();
	glDeleteTextures( 1, &m_texture );
	delete [] m_heightValuesNormalized;
	logbook::log_msg( logbook::TERRAIN, logbook::INFO,
			"Heightmap '" + m_filename + "' destroyed." );
}

void heightmap::bind() const {
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, m_texture );
}

void heightmap::unbind() const {
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, 0 );
}

}
