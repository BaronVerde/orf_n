
#include <base/logbook.h>
#include <renderer/texture.h>

namespace orf_n {

texture::texture( const GLenum target, const GLuint unit ) :
		m_target{ target }, m_unit{ unit } {
	if( unit == 20 )
		logbook::log_msg( logbook::RENDERER, logbook::WARNING,
				"Warning: texture unit 20 given for texture, but is used by UIOverlay !" );
	glCreateTextures( m_target, 1, &m_texture_name );
}

texture::~texture() {
	glDeleteTextures( 1, &m_texture_name );
}

void texture::bind_to_unit() const {
	glBindTextureUnit( m_unit, m_texture_name );
}

GLuint texture::get_unit() const {
	return m_unit;
}

GLuint texture::get_name() const {
	return m_texture_name;
}

const GLuint *texture::get_name_ptr() const {
	return &m_texture_name;
}


} /* namespace orf_n */
