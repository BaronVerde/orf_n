
/**
 * Base class to generally handle buffer objects.
 * @todo esclude copy and move
 */

#pragma once

#include "glad/glad.h"

namespace orf_n {

class Buffer {
public:

	typedef enum : unsigned int {
		STATIC_DATA = 0, DYNAMIC_DATA
	} bufferDataType_t;

	/**
	 * @brief Create a new buffer object.
	 * @param Target is one of:
	 * GL_ARRAY_BUFFER 				Vertex attributes
	 * GL_ATOMIC_COUNTER_BUFFER 	Atomic counter storage
	 * GL_COPY_READ_BUFFER 			Buffer copy source
	 * GL_COPY_WRITE_BUFFER 		Buffer copy destination
	 * GL_DISPATCH_INDIRECT_BUFFER 	Indirect compute dispatch commands
	 * GL_DRAW_INDIRECT_BUFFER 		Indirect command arguments
	 * GL_ELEMENT_ARRAY_BUFFER 		Vertex array indices
	 * GL_PIXEL_PACK_BUFFER 		Pixel read target
	 * GL_PIXEL_UNPACK_BUFFER 		Texture data source
	 * GL_QUERY_BUFFER 				Query result buffer
	 * GL_SHADER_STORAGE_BUFFER 	Read-write storage for shaders
	 * GL_TEXTURE_BUFFER 			Texture data buffer
	 * GL_TRANSFORM_FEEDBACK_BUFFER Transform feedback buffer
	 * GL_UNIFORM_BUFFER			Uniform block storage
	 * @param sizeInBytes Exactly that.
	 * @param data Pointer to data. If provided, bufferwill be initialized with data.
	 * @param flags Bitwise combination of
	 * GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT, GL_MAP_WRITE_BIT,
	 * GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT, GL_CLIENT_STORAGE_BIT
	 */
	Buffer( const GLenum target, const GLuint sizeInBytes, const void *data = nullptr,
			const GLbitfield flags = 0 );

	Buffer( const Buffer &other ) = delete;

	Buffer &operator=( const Buffer &other ) = delete;

	virtual ~Buffer();

	/**
	 * @brief Bind the bufer to its target.
	 */
	virtual void bind() const;

	/**
	 * @brief Bind 0 to the target.
	 */
	virtual void unBind() const;

	/**
	 * @brief Returns the buffer name. Other names can exist in inheriting classes.
	 */
	virtual const GLuint &getBufferName() const;

	/**
	 * @brief Returns whole buffer's size.
	 */
	virtual const GLuint &getSizeInBytes() const;

	/**
	 * @brief Map, fill and unmap part of the buffer with data.
	 *
	 * @param data Pointer the data to be copied
	 * @param size Use all of the buffer if 0 as set in ctor sizeInBytes
	 * @param offset In bytes from the beginning of the buffer
	 */
	virtual void updateSubData( const void *data, const GLuint size = 0, const GLint offset = 0 ) const;

	/**
	 * @todo
	 */
	void clearSubData() const;

	void copySubData() const;

private:
	GLuint m_bufferName{0};

	GLenum m_target{0};

	GLuint m_sizeInBytes{0};

	GLbitfield m_flags{0};

};

}
