
#include "program.h"
#include <stdexcept>
#include <iostream>

program::program( const std::vector<std::shared_ptr<module>> &modules ) {
    if( modules.size() == 0 ) {
    	std::cerr << "No shader modules specified for program." << std::endl;
    	throw std::runtime_error( "Shader module missing" );
    }
    m_program = glCreateProgram();
    if( !glIsProgram( m_program ) ) {
    	std::cerr << "Error creating shader program. Is not a program." << std::endl;
    	throw std::runtime_error( "Shader program creation failed" );
    }
    for( const std::shared_ptr<module> &m : modules ) {
        glAttachShader( m_program, m->getShader() );
        std::cout << "Attaching shader '" << m->getFilename() << "; #" << m->getShader() << '.' << std::endl;
    }
    link();
    std::cout << "Shader program #" << m_program << " linked. Ready for use" << std::endl;
}

program::~program() {
	glDeleteProgram( m_program );
	std::cout << "Shader program #" << m_program + " destroyed." << std::endl;
}

GLuint program::getProgram() const {
	return m_program;
}

void program::use() const {
	glUseProgram( m_program );
}

void program::unUse() const {
	glUseProgram( 0 );
}

void program::link() const {
	glLinkProgram( m_program );
	GLint linked;
	glGetProgramiv( m_program, GL_LINK_STATUS, &linked );
	if( !linked ) {
		GLsizei len;
		glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = new GLchar[len+1];
		glGetProgramInfoLog( m_program, len, &len, log );
		std::cerr << "Shader linkage failed: " << log << std::endl;
		delete [] log;
		glDeleteProgram( m_program );
		throw std::runtime_error( "Shader linkage failed" );
	}

}
