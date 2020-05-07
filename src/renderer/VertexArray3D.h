
/*
 * Vertex data for a 3D array can come in these flavours:
 * dvec3, hasHighLow == true: will be converted to 2 float interleaved
 * vec3, hasHighLow == true: high/low come interleaved
 * dvec3, hasHighLow == false: will be converted to vec3
 * vec3, hasHighLow == false: stays as it is
 * float, hasHighLow == false: dito
 * Does not normalize floating point data.
 * @todo: 16bit unsigned int for heightmaps.Delete copy and move
 */

#include <base/logbook.h>
#include "omath/vec3.h"
#include "Buffer.h"
#include <vector>
#include <typeinfo>
#include <sstream>
#include <fstream>
#include <memory>

#pragma once

namespace orf_n {

template<typename T>
class VertexArray3D {
public:
	// @todo all this is temporary and needs better handling
	const GLuint HIGH_ATTRIB_LOCATION{ 0 };
	const GLuint LOW_ATTRIB_LOCATION{ 1 };

	VertexArray3D( const std::vector<T> &vertexData,
				 const GLuint bindingIndex = 0,
				 const bool hasHighLowComponent = false,
				 const Buffer::bufferDataType_t type = Buffer::STATIC_DATA ) :
					 m_bindingIndex{ bindingIndex },
					 m_hasHighLowComponent{ hasHighLowComponent } {

		bool wrongDataType{ false };
		// create the buffer object
		glCreateVertexArrays( 1, &m_vaoName );
		// prepare data
		// @todo: the cases can be handled much shorter, but so it is better readable
		if( m_hasHighLowComponent ) {
			// data has prefab high and low component
			if( typeid( T ) == typeid( omath::vec3 ) ) {
				// case: high and low come interleaved -> can be used directly
				m_buffer = std::make_unique<Buffer>(
								GL_ARRAY_BUFFER, vertexData.size() * sizeof( omath::vec3 ),
								vertexData.data(),
								type == Buffer::STATIC_DATA ? 0x0 : GL_DYNAMIC_STORAGE_BIT
							);
			} else if( typeid( T ) == typeid( omath::dvec3 ) ) {
				// case: comes as double -> must be coverted before being sent to the buffer
				std::vector<omath::vec3> tempVertices( 2 * vertexData.size() );
				for( size_t i{0}; i < vertexData.size(); ++i )
					omath::double_to_two_floats( vertexData[i], tempVertices[2*i], tempVertices[2*i+1] );
				m_buffer = std::make_unique<Buffer>(
						GL_ARRAY_BUFFER, tempVertices.size() * sizeof( omath::vec3 ),
						tempVertices.data(),
						type == Buffer::STATIC_DATA ? 0x0 : GL_DYNAMIC_STORAGE_BIT
				);
			} else
				wrongDataType = true;

			// Bind the buffer and explain the attribute(s)
			if( !wrongDataType ) {
				// offset into buffer is 0 and stride is 2 * sizeof( vec3 )
				// Buffer offset is 0, stride between elements is 2 * vec3 for high and low component
				glVertexArrayVertexBuffer(
						m_vaoName, m_bindingIndex, m_buffer->getBufferName(), 0, 2 * sizeof( omath::vec3 )
				);
				// attributes for high and low location
				glVertexArrayAttribBinding( m_vaoName, HIGH_ATTRIB_LOCATION, m_bindingIndex );
				glVertexArrayAttribFormat( m_vaoName, HIGH_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, 0 );
				glVertexArrayAttribBinding( m_vaoName, LOW_ATTRIB_LOCATION, m_bindingIndex );
				glVertexArrayAttribFormat( m_vaoName, LOW_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GL_FLOAT ) );
				std::ostringstream s;
				s << "VertexArray3D '" << m_vaoName << "' created: received " <<
						vertexData.size() << " dvec3 converted to vec3 high/low interleaved values.";
				logbook::log_msg( logbook::RENDERER, logbook::INFO, s.str() );
			} else
				logbook::log_msg( logbook::RENDERER, logbook::ERROR,
						"Unexpected data type for VertexArray3D. Vertex array not created." );
		} else {
			if( typeid( T ) == typeid( omath::dvec3 ) ) {
				std::vector<omath::vec3> tempVertices( vertexData.size() );
				for( size_t i{0}; i < vertexData.size(); ++i )
					tempVertices[i] = omath::vec3{vertexData[i]};
				// single component, only 'high' location is set
				m_buffer = std::make_unique<Buffer>(
						GL_ARRAY_BUFFER, tempVertices.size() * sizeof( omath::vec3 ),
						tempVertices.data(),
						type == Buffer::STATIC_DATA ? 0x0 : GL_DYNAMIC_STORAGE_BIT
				);
				// offset into buffer is 0 and stride is sizeof( vec3 )
				glVertexArrayVertexBuffer(
						m_vaoName, m_bindingIndex, m_buffer->getBufferName(), 0, sizeof( omath::vec3 )
				);
				glVertexArrayAttribFormat( m_vaoName, 0, 3, GL_FLOAT, GL_FALSE, 0 );
				std::ostringstream s;
				s << "VertexArray3D '" << m_vaoName << "' created: " << vertexData.size() << " dvec3 values converted to vec3.";
				logbook::log_msg( logbook::RENDERER, logbook::INFO, s.str() );
			} else if( typeid( T ) == typeid( omath::vec3 ) ) {
				// single component, only 'high' location is set
				m_buffer = std::make_unique<Buffer>(
						GL_ARRAY_BUFFER, vertexData.size() * sizeof( omath::vec3 ),
						vertexData.data(),
						type == Buffer::STATIC_DATA ? 0x0 : GL_DYNAMIC_STORAGE_BIT
				);
				// offset into buffer is 0 and stride is sizeof( vec3 )
				glVertexArrayVertexBuffer(
						m_vaoName, m_bindingIndex, m_buffer->getBufferName(), 0, sizeof( omath::vec3 )
				);
				glVertexArrayAttribFormat( m_vaoName, 0, 3, GL_FLOAT, GL_FALSE, 0 );
				std::ostringstream s;
				s << "VertexArray3D '" << m_vaoName << "' created: " << vertexData.size() << " vec3 values.";
				logbook::log_msg( logbook::RENDERER, logbook::INFO, s.str() );
			} else if( typeid( T ) == typeid( float ) ) {// single component, only 'high' location is set
				m_buffer = std::make_unique<Buffer>(
						GL_ARRAY_BUFFER, vertexData.size() * sizeof( float ),
						vertexData.data(),
						type == Buffer::STATIC_DATA ? 0x0 : GL_DYNAMIC_STORAGE_BIT
				);
				// offset into buffer is 0 and stride is sizeof( vec3 )
				glVertexArrayVertexBuffer(
						m_vaoName, m_bindingIndex, m_buffer->getBufferName(), 0, sizeof( float )
				);
				glVertexArrayAttribFormat( m_vaoName, 0, 1, GL_FLOAT, GL_FALSE, 0 );
				std::ostringstream s;
				s << "VertexArray3D '" << m_vaoName << "' created: " << vertexData.size() << " float values.";
				logbook::log_msg( logbook::RENDERER, logbook::INFO, s.str() );
			} else {
				logbook::log_msg( logbook::RENDERER, logbook::ERROR,
						"Unexpected data type for VertexArray3D. Vertex array not created." );
			}
			glVertexArrayAttribBinding( m_vaoName, HIGH_ATTRIB_LOCATION, m_bindingIndex );
		}
		enable();
	}

	VertexArray3D( const VertexArray3D &v ) = delete;

	VertexArray3D &operator=( const VertexArray3D &v ) = delete;

	virtual ~VertexArray3D() {
		disable();
		glDeleteVertexArrays( 1, &m_vaoName );
		logbook::log_msg( logbook::RENDERER, logbook::INFO,
				"VertexArray3D " + std::to_string( m_vaoName ) + " destroyed." );
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
		m_buffer->updateSubData( data, size, offset );
	}

	void enable() const {
		glEnableVertexArrayAttrib( m_vaoName, HIGH_ATTRIB_LOCATION );
		if( m_hasHighLowComponent )
			glEnableVertexArrayAttrib( m_vaoName, LOW_ATTRIB_LOCATION );
	}

	void disable() const {
		glDisableVertexArrayAttrib( m_vaoName, HIGH_ATTRIB_LOCATION );
		if( m_hasHighLowComponent )
			glDisableVertexArrayAttrib( m_vaoName, LOW_ATTRIB_LOCATION );
	}

private:

	std::unique_ptr<Buffer> m_buffer{ nullptr };

	GLuint m_bindingIndex{ 0 };

	bool m_hasHighLowComponent{ false };

	GLuint m_vaoName{ 0 };

};

}
