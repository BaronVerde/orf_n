
/**
 * Base wrapper class for textures.
 *
 * @todo: format checks and loading are scattered over the derived classes ...
 */

#pragma once

#include "glad/glad.h"

namespace orf_n {

class texture {
public:
	texture( const GLenum target, const GLuint unit );

	texture( const texture &other ) = delete;

	texture &operator=( const texture &other ) = delete;

	virtual ~texture();

	/**
	 * @brief Bind texture to shader unit passed in on creation.
	 */
	virtual void bind_to_unit() const;

	virtual GLuint get_unit() const;

	virtual GLuint get_name() const;

	virtual const GLuint *get_name_ptr() const;

	/**
	 * @todo Discard the texture image.
	 */
	void invalidate_texture() const;

protected:
	GLenum m_target;

	GLuint m_texture_name;

	GLuint m_unit;

};

} /* namespace orf_n */
