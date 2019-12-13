
#pragma once

#include <omath/vec3.h>
#include <renderer/IndexBuffer.h>
#include <renderer/VertexArray3D.h>

namespace terrain {

class FlatMesh {
public:

	FlatMesh( const omath::uvec2 extent = omath::uvec2( 1024, 1024 ), bool clamped = false, GLuint bindingIndex = 0 );

	virtual ~FlatMesh();

	void bind() const;

	void unbind() const;

	GLuint getNumIndices() const;

private:
	/**
	 * Tile extent in x/z coordinates
	 */
	omath::uvec2 m_extent;

	std::vector<omath::vec3> m_vertices;

	GLuint m_numIndices;

	std::vector<GLuint> m_indices;

	GLuint m_bindingIndex;

	/**
	 * Position vertices in 2D. Displacement makes 3D.
	 */
	orf_n::VertexArray3D<omath::vec3> *m_vertexArray{ nullptr };

	orf_n::IndexBuffer *m_indexBuffer{ nullptr };

};

}
