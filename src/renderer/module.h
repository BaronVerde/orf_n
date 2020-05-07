
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

class module {
public:
	/**
	 * @brief Struct for shader modules
	 * @param type: In the following order:
	 * GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
	 * GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER
	 * @param filename Pathname to shader.
	 */
	module( const GLenum stage, const std::string &filename );

	module( const GLenum stage, const char** shader_source );

	module( const GLenum stage, const GLuint shader );

	virtual ~module();

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

	// Load a single standard glsl shader source file
	static std::vector<GLchar> load_shader_source( const std::string& filename );

	// Loads a source file. Scans for #include and loads the files named afetr that
	// Do not use <> or "" to bracket the filename, just plain whitespace !
	// Do not set clear_code to false if you don't want the code of the last loaded file.
	static std::vector<GLchar> parse_shader_source_file( const std::string& filename, bool clear_code = true );

	bool compile_shader( const char** shader_source );

};

}
