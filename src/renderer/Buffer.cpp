
#include "Buffer.h"

namespace orf_n {

Buffer::Buffer( const GLenum target, const GLuint sizeInBytes, const void *data,
				const GLbitfield flags ) :
				m_target{ target }, m_sizeInBytes{ sizeInBytes } {
	glCreateBuffers( 1, &m_bufferName );
	if( data == nullptr ) {
		glNamedBufferStorage( m_bufferName, sizeInBytes, nullptr, flags );
	} else {
		glNamedBufferStorage( m_bufferName, sizeInBytes, data, flags );
	}
	bind();
}

Buffer::~Buffer() {
	unBind();
	glDeleteBuffers( 1, &m_bufferName );
}

//virtual
void Buffer::bind() const {
	glBindBuffer( m_target, m_bufferName );
}

//virtual
void Buffer::unBind() const {
	glBindBuffer( m_target, 0 );
}

//virtual
const GLuint &Buffer::getBufferName() const {
	return m_bufferName;
}

//virtual
const GLuint &Buffer::getSizeInBytes() const {
	return m_sizeInBytes;
}

//virtual
void Buffer::updateSubData( const void *data, const GLuint size, const GLint offset ) const {
	if( size == 0 ) {
		glNamedBufferSubData( m_bufferName, offset, m_sizeInBytes, data );
	} else {
		glNamedBufferSubData( m_bufferName, offset, size, data );
	}
}

}
