
/*
 * vertexData can come in these flavours:
 * vec2, hasHighLow == false
 * float, hasHighLow == false
 * @todo Delete copy and move
 */

#include <base/Logbook.h>
#include <omath/vec3.h>
#include <renderer/Buffer.h>
#include <vector>
#include <typeinfo>
#include <sstream>
#include <memory>

#pragma once

namespace orf_n {

template<typename T>
class VertexArray2D {
public:
	VertexArray2D( const std::vector<T> &vertexData,
				 const GLuint bindingIndex = 0,
				 const Buffer::bufferDataType_t type = Buffer::STATIC_DATA ) :
					 m_bindingIndex{ bindingIndex } {

		// create the buffer object
		glCreateVertexArrays( 1, &m_vaoName );
		// prepare data
		m_buffer = std::make_unique<Buffer>(
				GL_ARRAY_BUFFER, vertexData.size() * sizeof( T ),
				vertexData.data(),
				type == Buffer::STATIC_DATA ? 0x0 : GL_DYNAMIC_STORAGE_BIT
		);
		glVertexArrayVertexBuffer(
				m_vaoName, m_bindingIndex, m_buffer->getBufferName(), 0, sizeof( T )
		);
		glVertexArrayAttribBinding( m_vaoName, 0, m_bindingIndex );
		if( typeid( T ) == typeid( omath::vec2 ) ) {
			glVertexArrayAttribFormat( m_vaoName, 0, 2, GL_FLOAT, GL_FALSE, 0 );
			std::ostringstream s;
			s << "VertexArray2D '" << m_vaoName << "' created: " << vertexData.size() << " vec2 values.";
			Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO, s.str() );
		} else if( typeid( T ) == typeid( float ) ) {
			glVertexArrayAttribFormat( m_vaoName, 0, 1, GL_FLOAT, GL_FALSE, 0 );
			std::ostringstream s;
			s << "VertexArray2D '" << m_vaoName << "' created: " << vertexData.size() << " float values.";
			Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO, s.str() );
		} else
			Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::ERROR,
					"Unexpected data type for VertexArray2D. Vertex array not created." );
	}

	VertexArray2D( const VertexArray2D &v ) = delete;

	VertexArray2D &operator=( const VertexArray2D &v ) = delete;

	virtual ~VertexArray2D() {
		disable();
		glDeleteVertexArrays( 1, &m_vaoName );
		Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::INFO,
				"VertexArray2D " + std::to_string( m_vaoName ) + " destroyed." );
	}

	/**
	 * @brief Binds the array to the current context.
	 */
	void bind() const {
		glBindVertexArray( m_vaoName );
	}

	void unBind() const {
		glBindVertexArray( 0 );
	}

	/**
	 * Copy data to the buffer if type was dynamicData during creation.
	 * Tyke care that data matches the one declared in ctor !
	 * @param data Pointer to the data to be copied
	 * @param size Use all of the buffer if 0, else size in byte
	 * @param offset In bytes from the beginning of the buffer
	 */
	void updateSubData( const void *data, const GLuint size, const GLint offset = 0 ) const {
		m_buffer->updateSubData( data, size,offset );
	}

	void enable() const {
		glEnableVertexArrayAttrib( m_vaoName, 0 );
	}

	void disable() const {
		glDisableVertexArrayAttrib( m_vaoName, 0 );
	}

private:

	std::unique_ptr<Buffer> m_buffer{ nullptr };

	GLuint m_bindingIndex{ 0 };

	bool m_hasHighLowComponent{ false };

	GLuint m_vaoName{ 0 };

};

}
