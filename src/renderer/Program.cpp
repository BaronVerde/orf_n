
#include <base/Logbook.h>
#include <renderer/Program.h>
#include <fstream>
#include <stdexcept>
#include <sstream>

namespace orf_n {

Program::Program( const std::vector<std::shared_ptr<Module>> &modules ) {
    if( modules.size() == 0 ) {
    	std::string s{ "No shader modules specified for program." };
    	Logbook::getInstance().logMsg( Logbook::SHADER, Logbook::ERROR, s );
    	throw std::runtime_error( s );
    }
    m_program = glCreateProgram();
    if( !glIsProgram( m_program ) ) {
    	std::string s{ "Error creating shader program. Is not a program." };
    	Logbook::getInstance().logMsg( Logbook::SHADER, Logbook::ERROR, s );
    	throw std::runtime_error( s );
    }
    for( const std::shared_ptr<Module> &m : modules ) {
        glAttachShader( m_program, m->getShader() );
        std::ostringstream s;
        s << "Attaching shader '" << m->getFilename() << "; #" << m->getShader() << '.';
        Logbook::getInstance().logMsg( Logbook::SHADER, Logbook::INFO, s.str() );
    }
    link();
    std::ostringstream s;
    s << "Shader program #" << m_program << " linked. Ready for use.";
    Logbook::getInstance().logMsg( Logbook::SHADER, Logbook::INFO, s.str() );
}

Program::~Program() {
	glDeleteProgram( m_program );
	Logbook::getInstance().logMsg( Logbook::SHADER, Logbook::INFO,
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
		Logbook::getInstance().logMsg( Logbook::SHADER, Logbook::ERROR, s );
		delete [] log;
		glDeleteProgram( m_program );
		throw std::runtime_error( s );
	}

}

}	// namespace
