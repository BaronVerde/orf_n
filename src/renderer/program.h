
#pragma once

#include "module.h"
#include <string>
#include <vector>
#include <memory>

namespace orf_n {

/**
 * A class for shader programs built from Modules (stages).
 */

class program {
public:
	/**
	 * @brief Create a new shaderprogram based on module objects.
	 * @param modules A vector of shared pointers. It is kept as long
	 * as the program is alive.
	 */
	program( const std::vector<std::shared_ptr<module>> &modules );

	virtual ~program();

	/**
	 * @brief Return the program name.
	 */
	GLuint getProgram() const;

	/**
	 * @brief Use the shader program as active program for rendering.
	 */
	void use() const;

	/**
	 * Binds 0 to the shader program binding point.
	 */
	void unUse() const;

private:
	/**
	 * Shader program handle as returned by glCreateProgram()
	 */
	GLuint m_program;

	void link() const;

};

}
