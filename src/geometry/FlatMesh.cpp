
#include <geometry/FlatMesh.h>
#include <renderer/VertexArray3D.h>
#include <vector>

namespace terrain {

using namespace orf_n;

FlatMesh::FlatMesh( const omath::uvec2 extent, bool clamped, GLuint bindingIndex ) :
			m_extent{ extent }, m_bindingIndex{ bindingIndex } {

	// Trivially compute 2D-positions x and z
	GLuint w{ m_extent.x + 1 };
	m_vertices.resize( w * ( m_extent.y + 1 ) );
	unsigned int i{ 0 };
	for( unsigned int y{ 0 }; y <= m_extent.y; ++y ) {
		for( unsigned int x{ 0 }; x <= m_extent.x; ++x ) {
			if( clamped )
				m_vertices[i++] = omath::vec3{
										static_cast<float>(x) / static_cast<float>(m_extent.x),
										0.0f,
										static_cast<float>(y) / static_cast<float>(m_extent.y)
								  };
			else
				m_vertices[i++] = omath::vec3{ static_cast<float>(x), 0.0f, static_cast<float>(y) };
		}
	}

	// Triangle indices, unsigned int. Front face points up on y axis, winding order CCW
	for( GLuint y{ 0 }; y < m_extent.y; ++y ) {
		for( GLuint x{ 0 }; x < m_extent.x; ++x ) {
			m_indices.push_back( x     + w * y );
			m_indices.push_back( x + 1 + w * y );
			m_indices.push_back( x + 1 + w * ( y + 1 ) );
			m_indices.push_back( x + 1 + w * ( y + 1 ) );
			m_indices.push_back( x     + w * ( y + 1 ) );
			m_indices.push_back( x     + w * y );
		}
	}

	// Create vertex array (binding index 0) and index buffer
	m_vertexArray = new VertexArray3D<omath::vec3>( m_vertices, m_bindingIndex );
	m_indexBuffer = new IndexBuffer( m_indices );
	m_numIndices = static_cast<GLuint>(m_indices.size());

}

GLuint FlatMesh::getNumIndices() const {
	return m_numIndices;
}

void FlatMesh::bind() const {
	m_vertexArray->bind();
	m_indexBuffer->bind();
}

void FlatMesh::unbind() const {
	m_vertexArray->unBind();
	m_indexBuffer->unBind();
}

FlatMesh::~FlatMesh() {
	delete m_vertexArray;
	delete m_indexBuffer;
}

}
