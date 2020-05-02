
#include "gridmesh.h"
#include "base/logbook.h"
#include "omath/vec3.h"
#include <vector>

namespace terrain {

gridmesh::gridmesh( const int dimension ) : m_dimension{ dimension } {
	int totalVertices = ( m_dimension + 1 ) * ( m_dimension + 1 );
	m_number_of_indices = m_dimension * m_dimension * 2 * 3;

	std::vector<omath::vec3> vertices( totalVertices );
	int vertexDimension = m_dimension + 1;
	for( int y = 0; y < vertexDimension; ++y )
		for( int x = 0; x < vertexDimension; ++x )
			vertices[x + vertexDimension * y] = omath::vec3{
													static_cast<float>(x) / static_cast<float>(m_dimension),
													0.0f,
													static_cast<float>(y) / static_cast<float>(m_dimension)
												};
	glCreateVertexArrays( 1, &m_vertexArray );
	glCreateBuffers( 1, &m_vertexBuffer );
	glNamedBufferData( m_vertexBuffer, vertices.size() * sizeof(omath::vec3), vertices.data(), GL_STATIC_DRAW );
	glVertexArrayVertexBuffer( m_vertexArray, VERTEX_BUFFER_BINDING_INDEX, m_vertexBuffer, 0, sizeof(omath::vec3) );
	glVertexArrayAttribBinding( m_vertexArray, 0, VERTEX_BUFFER_BINDING_INDEX );
	glVertexArrayAttribFormat( m_vertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0 );
	glEnableVertexArrayAttrib( m_vertexArray, 0 );
	std::vector<GLuint> indices( m_number_of_indices );
	int index = 0;
	int halfD = vertexDimension / 2;
	m_numberOfSubmeshIndices = halfD * halfD * 6;
	//Top Left
	for( int y = 0; y < halfD; y++ ) {
		for(int x = 0; x < halfD; x++) {
			indices[index++] = static_cast<GLuint>( x + vertexDimension * y );
			indices[index++] = static_cast<GLuint>( x + vertexDimension * (y + 1) );
			indices[index++] = static_cast<GLuint>( (x + 1) + vertexDimension * y );
			indices[index++] = static_cast<GLuint>( (x + 1) + vertexDimension * y );
			indices[index++] = static_cast<GLuint>( x + vertexDimension * (y + 1) );
			indices[index++] = static_cast<GLuint>( (x + 1) + vertexDimension * (y + 1) );
		}
	}
	m_endIndexTopLeft = index;
	//Top Right
	for(int y = 0; y < halfD;y++) {
		for(int x = halfD; x < m_dimension; x++) {
			indices[index++] = static_cast<GLuint>(x + vertexDimension * y);
			indices[index++] = static_cast<GLuint>(x + vertexDimension * (y + 1));
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * y);
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * y);
			indices[index++] = static_cast<GLuint>(x + vertexDimension * (y + 1));
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * (y + 1));
		}
	}
	m_endIndexTopRight = index;
	//Bottom Left
	for(int y = halfD; y < m_dimension;y++) {
		for(int x = 0; x < halfD; x++) {
			indices[index++] = static_cast<GLuint>(x + vertexDimension * y);
			indices[index++] = static_cast<GLuint>(x + vertexDimension * (y + 1));
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * y);
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * y);
			indices[index++] = static_cast<GLuint>(x + vertexDimension * (y + 1));
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * (y + 1));
		}
	}
	m_endIndexBottomLeft = index;
	//Bottom Right
	for(int y = halfD; y < m_dimension;y++) {
		for(int x = halfD; x < m_dimension; x++) {
			indices[index++] = static_cast<GLuint>(x + vertexDimension * y);
			indices[index++] = static_cast<GLuint>(x + vertexDimension * (y + 1));
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * y);
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * y);
			indices[index++] = static_cast<GLuint>(x + vertexDimension * (y + 1));
			indices[index++] = static_cast<GLuint>((x + 1) + vertexDimension * (y + 1));
		}
	}
	m_endIndexBottomRight = index;
	glCreateBuffers( 1, &m_indexBuffer );
	glNamedBufferData( m_indexBuffer, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW );
	glVertexArrayElementBuffer( m_vertexArray, m_indexBuffer );

	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO,
			"Gridmesh dimension " + std::to_string( m_dimension ) + " created." );
}

void gridmesh::bind() const {
	glBindVertexArray( m_vertexArray );
}

void gridmesh::unbind() const {
	glBindVertexArray( 0 );
}

gridmesh::~gridmesh() {
	glDisableVertexArrayAttrib( m_vertexArray, 0 );
	glDeleteBuffers( 1, &m_indexBuffer );
	glDeleteBuffers( 1, &m_vertexBuffer );
	glDeleteVertexArrays( 1, &m_vertexArray );
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO,
			"Gridmesh dimension " + std::to_string( m_dimension ) + " destroyed." );
}

int gridmesh::getDimension() const {
	return m_dimension;
}

int gridmesh::getEndIndexTL() const {
	return m_endIndexTopLeft;
}

int gridmesh::getEndIndexTR() const {
	return m_endIndexTopRight;
}

int gridmesh::getEndIndexBL() const {
	return m_endIndexBottomLeft;
}

int gridmesh::getEndIndexBR() const {
	return m_endIndexBottomRight;
}

int gridmesh::getNumberOfSubMeshIndices() const {
	return m_numberOfSubmeshIndices;
}

GLsizei gridmesh::get_number_indices() const {
	return m_number_of_indices;
}

}
