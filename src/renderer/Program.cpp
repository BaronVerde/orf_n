
#include <base/logbook.h>
#include "Program.h"
#include <fstream>
#include <stdexcept>
#include <sstream>

namespace orf_n {

Program::Program( const std::vector<std::shared_ptr<Module>> &modules ) {
    if( modules.size() == 0 ) {
    	std::string s{ "No shader modules specified for program." };
    	logbook::log_msg( logbook::SHADER, logbook::ERROR, s );
    	throw std::runtime_error( s );
    }
    m_program = glCreateProgram();
    if( !glIsProgram( m_program ) ) {
    	std::string s{ "Error creating shader program. Is not a program." };
    	logbook::log_msg( logbook::SHADER, logbook::ERROR, s );
    	throw std::runtime_error( s );
    }
    for( const std::shared_ptr<Module> &m : modules ) {
        glAttachShader( m_program, m->getShader() );
        std::ostringstream s;
        s << "Attaching shader '" << m->getFilename() << "; #" << m->getShader() << '.';
        logbook::log_msg( logbook::SHADER, logbook::INFO, s.str() );
    }
    link();
    std::ostringstream s;
    s << "Shader program #" << m_program << " linked. Ready for use.";
    logbook::log_msg( logbook::SHADER, logbook::INFO, s.str() );
}

Program::~Program() {
	glDeleteProgram( m_program );
	logbook::log_msg( logbook::SHADER, logbook::INFO,
			"Shader program #" + std::to_string( m_program ) + " destroyed." );
}

GLuint Program::getProgram() const {
	return m_program;
}

void Program::use() const {
	glUseProgram( m_program );
}

void Program::unUse() const {
	glUseProgram( 0 );
}

void Program::link() const {
	glLinkProgram( m_program );
	GLint linked;
	glGetProgramiv( m_program, GL_LINK_STATUS, &linked );
	if( !linked ) {
		GLsizei len;
		glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = new GLchar[len+1];
		glGetProgramInfoLog( m_program, len, &len, log );
		std::string s{ "Shader linkage failed: " + std::string{ log } };
		logbook::log_msg( logbook::SHADER, logbook::ERROR, s );
		delete [] log;
		glDeleteProgram( m_program );
		throw std::runtime_error( s );
	}

}

}	// namespace
