
#include "base/Logbook.h"
#include "Framebuffer.h"
#include "iostream"

namespace orf_n {

Framebuffer::Framebuffer( const unsigned int x, const unsigned int y ) :
		m_sizeX{ x }, m_sizeY{ y } {
	glCreateFramebuffers( 1, &m_framebuffer );
	if( GL_TRUE != glIsFramebuffer( m_framebuffer ) ) {
		std::string s{ "Error creating framebuffer object." };
		Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO, s );
		throw std::runtime_error( s );
	}
	Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO,
			"Created framebuffer object " + std::to_string( m_framebuffer ) );
}

Framebuffer::~Framebuffer() {
	if( GL_TRUE == glIsRenderbuffer( m_depthAttachment ) )
		glDeleteRenderbuffers( 1, &m_depthAttachment );
	if( GL_TRUE == glIsRenderbuffer( m_colorAttachment ) )
		glDeleteRenderbuffers( 1, &m_colorAttachment );
	glDeleteFramebuffers( 1, &m_framebuffer );
	Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO,
			"Framebuffer object " + std::to_string( m_framebuffer ) + " destroyed.");
}

void Framebuffer::addColorAttachment( const GLenum colorFormat, GLenum attachmentPoint ) {
	glCreateRenderbuffers( 1, &m_colorAttachment );
	if( GL_TRUE != glIsRenderbuffer( m_colorAttachment ) )
		std::cerr << "Error creating color renderbuffer.\n";
	glNamedRenderbufferStorage( m_colorAttachment, colorFormat, m_sizeX, m_sizeY );
	glNamedFramebufferRenderbuffer( m_framebuffer, attachmentPoint, GL_RENDERBUFFER, m_colorAttachment );
}

void Framebuffer::addDepthAttachment( const GLenum depthFormat ) {
	glCreateRenderbuffers( 1, &m_depthAttachment );
	if( GL_TRUE != glIsRenderbuffer( m_depthAttachment ) )
		std::cerr << "Error creating depth renderbuffer.\n";
	glNamedRenderbufferStorage( m_depthAttachment, depthFormat, m_sizeX, m_sizeY );
	glNamedFramebufferRenderbuffer( m_framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

bool Framebuffer::isComplete() const {
	return GL_FRAMEBUFFER_COMPLETE != glCheckNamedFramebufferStatus( m_framebuffer, GL_FRAMEBUFFER ) ? false : true;
}

void Framebuffer::bind( const GLenum target ) const {
	glBindFramebuffer( target, m_framebuffer );
}

void Framebuffer::resize( const unsigned int x, const unsigned int y ) {
	m_sizeX = x;
	m_sizeY = y;
	if( GL_TRUE == glIsRenderbuffer( m_colorAttachment ) ) {
		//glInvalidateNamedFramebufferData( m_framebuffer, 1, &m_colorAttachment );
		glDeleteRenderbuffers( 1, &m_colorAttachment );
		addColorAttachment( m_colorFormat );
	}
	if( GL_TRUE == glIsRenderbuffer( m_depthAttachment ) ) {
		//glInvalidateNamedFramebufferData( m_framebuffer, 1, &m_depthAttachment );
		glDeleteRenderbuffers( 1, &m_depthAttachment );
		addDepthAttachment( m_depthFormat );
	}
}

void Framebuffer::clear( const color_t &clearColor ) const {
	glClearColor( clearColor.x, clearColor.y, clearColor.z, clearColor.w );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

}
