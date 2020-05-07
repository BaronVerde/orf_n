
#pragma once

//#include "base/Globals.h"
#include "omath/mat4.h"
#include <string>
#include "glad/glad.h"

/**
 * Helper functions to set uniform not belonging to the convention above
 * assuming that vectors and matrices are stored as c-arrays without padding
 * like is the case with omath's value_ptr.
 *
 * Supported types: bool, GLint, GLuint, GLfloat, GLdouble,
 * omath::vec2, omath::vec3, omath::vec4, omath::dvec2, omath::dvec3, omath::dvec4,
 * omath::mat2, omath::mat3, omath::mat4, omath::dmat2, omath::dmat3, omath::dmat4,
 * omath::dmat42
 */

namespace orf_n {

/**
 * Shader uniform locations for standard uniforms to avoid many lookups.
 * If a shader uses one or more of these uniforms it must declare them
 * under these uniform locations in order to be directly accessible
 * via the below static functions.
 * SUL_STANDARD_BASE ist just the base for enumerating. Don't change ;-)
 */
const GLint SUL_CAMERA_POSITION_HIGH { 5 };
const GLint SUL_CAMERA_POSITION_LOW { 6 };
const GLint SUL_MODEL_MATRIX{ 7 };
const GLint SUL_VIEW_MATRIX{ 8 };
const GLint SUL_MODEL_VIEW_MATRIX{ 9 };
const GLint SUL_MODEL_VIEW_PROJECTION_MATRIX{ 10 };
const GLint SUL_PROJECTION_MATRIX{ 11 };
const GLint SUL_SUN_DIRECTION{ 12 };
const GLint SUL_SUN_COLOR{ 13 };
const GLint SUL_DIFFUSE_SPECULAR_AMBIENT_SHININESS{ 14 };
const GLint SUL_VIEW_PROJECTION_MATRIX{ 15 };
const GLint SUL_TERRAIN_HEIGHT_HIGH { 16 };
const GLint SUL_TERRAIN_HEIGHT_LOW { 17 };
// model view projection matrix relative to eye, view matrix stripped of translation
const GLint SUL_MODEL_VIEW_PROJECTION_MATRIX_RTE { 18 };

static inline void setCameraPosition( const omath::dvec3 &pos ) {
	omath::vec3 h{ 0.0f }, l{  0.0f };
	double_to_two_floats( pos, h, l );
	glUniform3fv( SUL_CAMERA_POSITION_HIGH, 1, &h[0] );
	glUniform3fv( SUL_CAMERA_POSITION_LOW, 1, &l[0] );
}

static inline void setCameraPosition( const omath::vec3 &pos ) {
	glUniform3fv( SUL_CAMERA_POSITION_HIGH, 1, &pos[0] );
}

static inline void setModelMatrix( const omath::mat4 &mm ) {
	glUniformMatrix4fv( SUL_MODEL_MATRIX, 1, GL_FALSE, &mm[0][0] );
}

static inline void setViewMatrix( const omath::mat4 &vm ) {
	glUniformMatrix4fv( SUL_VIEW_MATRIX, 1, GL_FALSE, &vm[0][0] );
}

static inline void setProjectionMatrix( const omath::mat4 &pm ) {
	glUniformMatrix4fv( SUL_PROJECTION_MATRIX, 1, GL_FALSE, &pm[0][0] );
}

static inline void setModelViewMatrix( const omath::mat4 &vm ) {
	glUniformMatrix4fv( SUL_MODEL_VIEW_MATRIX, 1, GL_FALSE, &vm[0][0] );
}

static inline void setModelViewProjectionMatrix( const omath::mat4 &pm ) {
	glUniformMatrix4fv( SUL_MODEL_VIEW_PROJECTION_MATRIX, 1, GL_FALSE, &pm[0][0] );
}

static inline void setModelViewProjectionMatrixRTE( const omath::mat4 &pm ) {
	glUniformMatrix4fv( SUL_MODEL_VIEW_PROJECTION_MATRIX_RTE, 1, GL_FALSE, &pm[0][0] );
}

static inline void setViewProjectionMatrix( const omath::mat4 &pm ) {
	glUniformMatrix4fv( SUL_VIEW_PROJECTION_MATRIX, 1, GL_FALSE, &pm[0][0] );
}

static inline void setSunDirection( const omath::vec3 &lpos ) {
	glUniform3fv( SUL_SUN_DIRECTION, 1, &lpos[0] );
}

static inline void setSunColor( const omath::vec3 &col ) {
	glUniform3fv( SUL_SUN_COLOR, 1, &col[0] );
}

static inline void setDiffuseSpecularAmbientShininess( const omath::vec4 &dsas ) {
	glUniform4fv( SUL_DIFFUSE_SPECULAR_AMBIENT_SHININESS, 1, &dsas[0] );
}

// Generic uniform setters for shader vars. Not the fastest ...
static inline void set_uniform( const GLuint program, const std::string &name, bool value ) {
	glUniform1i( glGetUniformLocation( program, name.c_str() ), (GLint)value );
}

/**
 * @brief This is the right overload for textures. Despite they being declared as GLuint
 * the api expects a GLint. So cast the texture before calling setUniform().
 */
static inline void set_uniform( const GLuint program, const std::string &name, const GLint &value ) {
	glUniform1i( glGetUniformLocation( program, name.c_str() ), value );
}

static inline void set_uniform( const GLuint program, const std::string &name, const GLuint &value ) {
	glUniform1ui( glGetUniformLocation( program, name.c_str() ), value );
}

static inline void set_uniform( const GLuint program, const std::string &name, const GLfloat &value ) {
	glUniform1f( glGetUniformLocation( program, name.c_str() ), value );
}

static inline void set_uniform( const GLuint program, const std::string &name, const GLdouble &value ) {
	glUniform1d( glGetUniformLocation( program, name.c_str() ), value );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::vec2 &value ) {
	glUniform2fv( glGetUniformLocation( program, name.c_str() ), 1, &value[0] );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::vec3 &value ) {
	glUniform3fv( glGetUniformLocation( program, name.c_str() ), 1, &value[0] );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::vec4 &value ) {
	glUniform4fv(glGetUniformLocation( program, name.c_str() ), 1, &value[0] );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::dvec2 &value ) {
	glUniform2dv( glGetUniformLocation( program, name.c_str() ), 1, &value[0] );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::dvec3 &value ) {
	glUniform3dv( glGetUniformLocation( program, name.c_str() ), 1, &value[0] );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::dvec4 &value ) {
	glUniform4dv(glGetUniformLocation( program, name.c_str() ), 1, &value[0] );
}

/**
 * @todo: Try omath::value_ptr(trans) as last argument.
 * omath stores matrices not in the way OpenGL expects them.
 */
/*static inline void set_uniform( const GLuint program, const std::string &name, const omath::mat2 &mat ) {
	glUniformMatrix2fv( glGetUniformLocation( program, name.c_str() ), 1, GL_FALSE, &mat[0][0] );
}*/

static inline void set_uniform( const GLuint program, const std::string &name, const omath::mat3 &mat ) {
	glUniformMatrix3fv( glGetUniformLocation( program, name.c_str() ), 1, GL_FALSE, &mat[0][0] );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::mat4 &mat ) {
	glUniformMatrix4fv( glGetUniformLocation( program, name.c_str() ), 1, GL_FALSE, &mat[0][0] );
}

/*static inline void set_uniform( const GLuint program, const std::string &name, const omath::dmat2 &mat ) {
	glUniformMatrix2dv( glGetUniformLocation( program, name.c_str() ), 1, GL_FALSE, &mat[0][0] );
}*/

static inline void set_uniform( const GLuint program, const std::string &name, const omath::dmat3 &mat ) {
	glUniformMatrix3dv( glGetUniformLocation( program, name.c_str() ), 1, GL_FALSE, &mat[0][0] );
}

static inline void set_uniform( const GLuint program, const std::string &name, const omath::dmat4 &mat ) {
	glUniformMatrix4dv( glGetUniformLocation( program, name.c_str() ), 1, GL_FALSE, &mat[0][0] );
}

/**
 * @todo This might be 2x4 ...
 */
/*static inline void setUniform( const GLuint program, const std::string &name, const omath::dmat4x2 &mat ) {
	glUniformMatrix4x2dv( glGetUniformLocation( program, name.c_str() ), 1, GL_FALSE, &mat[0][0] );
}*/

}
