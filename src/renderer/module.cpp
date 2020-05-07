
#include "module.h"
#include <fstream>
#include <iostream>
#include <cstdio>

namespace orf_n {

module::module( const GLenum stage, const std::string &filename ) :
		m_filename( filename ), m_stage( stage ), m_isShader( false ) {
	std::vector<GLchar> source{ load_shader_source( m_filename ) };
	if( !source.empty() ) {
		m_shader = glCreateShader( stage );
		const GLchar *x{ source.data() };
		if( compile_shader( &x ) ) {
			m_isShader = true;
			std::cout << "Loaded and compiled shader '" << m_filename << "'; #" << m_shader << ".\n";
		} else
			std::cerr << "Shader " << m_filename << " compilation failed.\n";
	} else
		std::cerr << "Shader " << m_filename << " not loaded.\n";
}

module::module( const GLenum stage, const char** shader_source ) : m_stage{stage} {
	m_shader = glCreateShader( stage );
	if( compile_shader( shader_source ) ) {
		m_isShader = true;
		std::cout << "Compiled shader #" << m_shader << ".\n";
	} else
		std::cerr << "Shader compilation failed.\n";
}

module::module( const GLenum stage, const GLuint shader ) :
		m_shader{shader}, m_stage{stage} {
	m_isShader = glIsShader( shader ) ? true : false;
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

bool module::compile_shader( const char** shader_source ) {
	// Source assumed to be null-terminated
	glShaderSource( m_shader, 1, shader_source, NULL );
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
		return false;
	}
	return true;
}

// static
std::vector<GLchar> module::load_shader_source( const std::string& filename ) {
	std::ifstream file{ filename, std::ios::ate | std::ios::binary };
	if( !file.is_open() ) {
		std::cerr << "Unable to open shader file '" + filename + "'" << std::endl;
		throw std::runtime_error( "Shader file not found" );
	}
	size_t fileSize{ (size_t)file.tellg() };
	std::vector<GLchar> buffer( fileSize + 1 );
	file.seekg(0);
	file.read( buffer.data(), fileSize );
	file.close();
	buffer[fileSize] = '\0';
	return buffer;
}	// readFile()

// static
std::vector<GLchar> module::parse_shader_source_file( const std::string& filename, bool clear_code ) {
	// static to accumulate over recursive calls
	static std::vector<std::string> source_code;
	if( clear_code )
		source_code.clear();
	std::ifstream file{ filename, std::ios::in };
	if( file ) {
		std::string line;
		char found_filename[50];
		while( getline( file, line ) ) {
			if( 1 == sscanf( line.c_str(), "#include %49s", found_filename ) )
				parse_shader_source_file( found_filename, false );
			else
				source_code.push_back( line + '\n' );
		}
		file.close();
	}
	std::vector<GLchar> retval;
	for( const std::string& s : source_code )
		std::copy( s.begin(), s.end(), std::back_inserter( retval ) );
	return retval;
}

}
