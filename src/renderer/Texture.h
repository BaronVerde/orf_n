
/**
 * Base wrapper class for textures.
 *
 * @todo: format checks and loading are scattered over the derived classes ...
 */

#pragma once

#include "../../extern/glad/glad.h"

namespace orf_n {

class Texture {
public:
	Texture( const GLenum target, const GLuint unit );

	Texture( const Texture &other ) = delete;

	Texture &operator=( const Texture &other ) = delete;

	virtual ~Texture();

	/**
	 * @brief Bind texture to shader unit passed in on creation.
	 */
	virtual void bindToUnit() const;

	virtual GLuint getUnit() const;

	virtual GLuint getName() const;

	virtual const GLuint *getNamePtr() const;

	/**
	 * @todo Discard the texture image.
	 */
	void invalidateTexture() const;

protected:
	GLenum m_target;

	GLuint m_textureName;

	GLuint m_unit;

};

} /* namespace orf_n */
