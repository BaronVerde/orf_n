
#include <base/logbook.h>
#include "Module.h"
#include <sstream>
#include <fstream>

namespace orf_n {

Module::Module( const GLenum stage, const std::string &filename ) :
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
		logbook::log_msg( logbook::SHADER, logbook::ERROR, "Shader " + m_filename + " compilation failed: " + log );
		delete [] log;
		glDeleteShader( m_shader );
		throw std::runtime_error( "Shader compilation error. See logfile." );
	}
	m_isShader = true;
	std::ostringstream s;
	s << "Loaded shader '" << m_filename << "'; #" << m_shader << '.';
	logbook::log_msg( logbook::SHADER, logbook::INFO, s.str() );
	// shader sourcecode is not needed any more.
}

Module::Module( const GLenum stage, const GLchar *moduleText[] ) {
	m_shader = glCreateShader( stage );
	// Source assumed to be null-terminated
	glShaderSource( m_shader, 1, moduleText, NULL );
	glCompileShader( m_shader );
	GLint compiled;
	glGetShaderiv( m_shader, GL_COMPILE_STATUS, &compiled );
	if( !compiled ) {
		GLsizei len;
		glGetShaderiv( m_shader, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = new GLchar[len+1];
		glGetShaderInfoLog( m_shader, len, &len, log );
		logbook::log_msg( logbook::SHADER, logbook::ERROR, "Shader " + m_filename + " compilation failed: " + log );
		delete [] log;
		glDeleteShader( m_shader );
		throw std::runtime_error( "Shader compilation error. See logfile." );
	}
	m_isShader = true;
	std::ostringstream s;
	s << "Loaded shader '" << m_filename << "'; #" << m_shader << '.';
	logbook::log_msg( logbook::SHADER, logbook::INFO, s.str() );
}

Module::~Module() {
	if( m_isShader )
		glDeleteShader( m_shader );
	logbook::log_msg( logbook::SHADER, logbook::INFO,
			"Shader #" + std::to_string( m_shader ) + " destroyed." );
}

GLuint Module::getShader() const {
	return m_shader;
}

bool Module::isShader() const {
	return m_isShader;
}

const std::string &Module::getFilename() const {
	return m_filename;
}

// static
std::vector<GLchar> Module::readShaderSourceFile( const std::string& filename ) {
	std::ifstream file( filename, std::ios::ate | std::ios::binary );
	if( !file.is_open() ) {
		std::string s{ "Unable to open shader file '" + filename + "'." };
		logbook::log_msg( logbook::SHADER, logbook::ERROR, s );
		throw std::runtime_error( s );
	}

	size_t fileSize{ (size_t)file.tellg() };
	std::vector<char> buffer( fileSize + 1 );
	file.seekg(0);
	file.read( buffer.data(), fileSize );
	file.close();
	buffer[fileSize] = '\0';
	return buffer;
}	// readFile()


}
