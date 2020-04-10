
#include "module.h"
#include <fstream>
#include <iostream>

module::module( const GLenum stage, const std::string &filename ) :
		m_filename( filename ), m_stage( stage ), m_isShader( false ) {
	std::vector<GLchar> source = readShaderSourceFile( m_filename );
	m_shader = glCreateShader( stage );
	const GLchar *x{ source.data() };
	// Source assumed to be null-terminated
	glShaderSource( m_shader, 1, &x, NULL );
	glCompileShader( m_shader );
	GLint compiled;
	glGetShaderiv( m_shader, GL_COMPILE_STATUS, &compiled );
	if( !compiled ) {
		GLsizei len;
		glGetShaderiv( m_shader, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = new GLchar[len+1];
		glGetShaderInfoLog( m_shader, len, &len, log );
		std::cerr << "Shader '" << m_filename << "' compilation failed:\n" << log << std::endl;
		delete [] log;
		glDeleteShader( m_shader );
		throw std::runtime_error( "Shader compilation error" );
	}
	m_isShader = true;
	std::cout << "Loaded shader '" << m_filename << "'; #" << m_shader << ".\n";
}

module::~module() {
	if( m_isShader )
		glDeleteShader( m_shader );
	std::cout << "Shader #" << m_shader << " destroyed\n";
}

GLuint module::getShader() const {
	return m_shader;
}

bool module::isShader() const {
	return m_isShader;
}

const std::string &module::getFilename() const {
	return m_filename;
}

// static
std::vector<GLchar> module::readShaderSourceFile( const std::string& filename ) {
	std::ifstream file( filename, std::ios::ate | std::ios::binary );
	if( !file.is_open() ) {
		std::cerr << "Unable to open shader file '" + filename + "'" << std::endl;
		throw std::runtime_error( "Shader file not found" );
	}
	size_t fileSize{ (size_t)file.tellg() };
	std::vector<char> buffer( fileSize + 1 );
	file.seekg(0);
	file.read( buffer.data(), fileSize );
	file.close();
	buffer[fileSize] = '\0';
	return buffer;
}	// readFile()
