
/**
 * Texture cubes.
 * @todo: OpenGL 4.5 style !
 */

#pragma once

#include <renderer/texture.h>
#include <vector>
#include <string>
#include "glad/glad.h"

namespace orf_n {

/**
 * A 6 sided texture cube for sky and cube maps.
 * Will be bound to samplerCube.
 * @todo if more than 1.
 */
class TextureCube {
public:
	/**
	 * Create a new cube map.
	 * Pass in array of filenames and shader unit to bind the texture to.
	 */
	TextureCube( const std::vector<std::string> &files );

	TextureCube( const TextureCube &other ) = delete;

	TextureCube &operator=( const TextureCube &other ) = delete;

	/**
	 * Bind cube map to its binding point.
	 */
	void bind() const;

	virtual ~TextureCube();

private:
	GLuint m_textureName;

};

}
