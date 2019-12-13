
#include <renderer/IndexBuffer.h>

namespace orf_n {

IndexBuffer::IndexBuffer( const std::vector<GLuint> &indexData ) :
	Buffer( GL_ELEMENT_ARRAY_BUFFER,
			(GLuint)( indexData.size() * sizeof(GLuint) ),
			indexData.data(),
			0 ),
			m_numberOfElements { (GLuint)indexData.size() } {}

// virtual
IndexBuffer::~IndexBuffer() {}

const GLuint &IndexBuffer::getNumber() const {
	return m_numberOfElements;
}

}	// namespace
