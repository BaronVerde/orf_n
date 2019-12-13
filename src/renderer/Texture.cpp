
#include <base/Logbook.h>
#include <renderer/Texture.h>

namespace orf_n {

Texture::Texture( const GLenum target, const GLuint unit ) :
		m_target{ target }, m_unit{ unit } {
	if( unit == 20 )
		Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::WARNING,
				"Warning: texture unit 20 given for texture, but is used by UIOverlay !" );
	glCreateTextures( m_target, 1, &m_textureName );
}

Texture::~Texture() {
	glDeleteTextures( 1, &m_textureName );
}

void Texture::bindToUnit() const {
	glBindTextureUnit( m_unit, m_textureName );
}

GLuint Texture::getUnit() const {
	return m_unit;
}

GLuint Texture::getName() const {
	return m_textureName;
}

const GLuint *Texture::getNamePtr() const {
	return &m_textureName;
}


} /* namespace orf_n */
