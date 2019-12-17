
#pragma once

#include <string>
#include <vector>
#include "glad/glad.h"

/**
 * @brief Class for a shader module for use in a pipeline
 * Can be a vertex-, geometry-, tesscontrol, tessevaluation or fragment
 * shadermodule.
 * @todo: resource ! If module exists, return existing !
 */

namespace orf_n {

class Module {
public:
	/**
	 * @brief Struct for shader modules
	 * @param type: In the following order:
	 * GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
	 * GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER
	 * @param filename Pathname to shader.
	 */
	Module( const GLenum stage, const std::string &filename );

	/**
	 * Same as above, only the text comes as a char* (e.g. inline)
	 */
	Module( const GLenum stage, const GLchar *moduleText[] );

	virtual ~Module();

	/**
	 * Return the shader stage for binding to a pipeline.
	 */
	GLuint getShader() const;

	/**
	 * Is it a valid shader module for use in Pipeline ?
	 */
	bool isShader() const;

	const std::string &getFilename() const;

private:
	/**
	 * The shader stage.
	 */
	GLuint m_shader;

	std::string m_filename;

	/**
	 * Type of the shader stage.
	 */
	GLenum m_stage;

	/**
	 * Is it a valid shader stage, or has compilation failed.
	 */
	bool m_isShader;

	/**
	 * static helper function
	 */
	static std::vector<GLchar> readShaderSourceFile( const std::string &filename );

};

} /* namespace orf_n */
