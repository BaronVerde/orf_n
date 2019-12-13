
/**
 * @brief Class for index (element) buffers. Only GLuint as index type is supported.
 *
 * Creates its own buffer object.
 *
 * @todo: Other datatypes and composed index buffers with sub storage. Delete copy and move
 */
#pragma once

#include <renderer/Buffer.h>

#include <vector>
#include "../../extern/glad/glad.h"

namespace orf_n {

class IndexBuffer : public Buffer {
public:
	/**
	 * @brief Create new index buffer instance.
	 *
	 * @param indexData: Vector to the index data of type GLuint.
	 */
	IndexBuffer( const std::vector<GLuint> &indexData );

	IndexBuffer( const IndexBuffer &other ) = delete;

	IndexBuffer &operator=( const IndexBuffer &other ) = delete;

	virtual ~IndexBuffer();

	/**
	 * @brief Return number of index elements.
	 */
	const GLuint &getNumber() const;

private:
	GLuint m_numberOfElements;

	/**
	 * This is fixed for now.
	 */
	const GLenum m_dataType{ GL_UNSIGNED_INT };

};

}	// namespace
