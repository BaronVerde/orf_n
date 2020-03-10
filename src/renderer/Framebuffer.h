
/**
 * WIP
 */

#pragma once

#include "Color.h"
#include "glad/glad.h"

namespace orf_n {

class Framebuffer {
public:
	/**
	 * Create a framebuffer object with attachments of given size and bound to target.
	 */
	Framebuffer( const unsigned int x, const unsigned int y );

	virtual ~Framebuffer();

	void bind( const GLenum target ) const;

	void addColorAttachment( const GLenum colorFormat = GL_SRGB, GLenum attachmentPoint = GL_COLOR_ATTACHMENT0 );

	void addDepthAttachment( const GLenum depthFormat = GL_DEPTH_COMPONENT32F );

	void resize( const unsigned int x, const unsigned int y );

	bool isComplete() const;

	/**
	 * Clear color to black and depth buffer;
	 */
	void clear( const color_t &clearColor = color::black ) const;

private:
	unsigned int m_sizeX;

	unsigned int m_sizeY;

	GLuint m_framebuffer{ 0 };

	GLuint m_colorAttachment{ 0 };

	GLenum m_colorFormat{ GL_SRGB };

	GLenum m_depthFormat{ GL_DEPTH_COMPONENT32F };

	GLuint m_depthAttachment{ 0 };

	GLuint m_stencilAttachment{ 0 };

};

}
