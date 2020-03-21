
#include <base/logbook.h>
#include "GridMesh.h"
#include "renderer/VertexArray3D.h"
#include <vector>

namespace terrain {

GridMesh::GridMesh( const int dimension ) : m_dimension{ dimension } {
	int totalVertices = ( m_dimension + 1 ) * ( m_dimension + 1 );
	int totalIndices = m_dimension * m_dimension * 2 * 3;

	std::vector<omath::vec3> vertices( totalVertices );
	int vertexDimension = m_dimension + 1;
	for( int y = 0; y < vertexDimension; ++y )
		for( int x = 0; x < vertexDimension; ++x )
			vertices[x + vertexDimension * y] = omath::vec3{
													static_cast<float>(x) / static_cast<float>(m_dimension),
													0.0f,
													static_cast<float>(y) / static_cast<float>(m_dimension)
												};
	m_vertexArray = new orf_n::VertexArray3D<omath::vec3>{ vertices, 0 };
	std::vector<GLuint> indices( totalIndices );
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
	m_indexBuffer = new orf_n::IndexBuffer( indices );

	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO,
			"Gridmesh dimension " + std::to_string( m_dimension ) + " created." );
}

void GridMesh::bind() const {
	m_vertexArray->bind();
	m_indexBuffer->bind();
}

void GridMesh::unbind() const {
	m_vertexArray->unBind();
	m_indexBuffer->unBind();
}

GridMesh::~GridMesh() {
	delete m_indexBuffer;
	delete m_vertexArray;
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO,
			"Gridmesh dimension " + std::to_string( m_dimension ) + " destroyed." );
}

int GridMesh::getDimension() const {
	return m_dimension;
}

int GridMesh::getEndIndexTL() const {
	return m_endIndexTopLeft;
}

int GridMesh::getEndIndexTR() const {
	return m_endIndexTopRight;
}

int GridMesh::getEndIndexBL() const {
	return m_endIndexBottomLeft;
}

int GridMesh::getEndIndexBR() const {
	return m_endIndexBottomRight;
}

int GridMesh::getNumberOfSubMeshIndices() const {
	return m_numberOfSubmeshIndices;
}

int GridMesh::getNumberOfIndices() const {
	return m_indexBuffer->getNumber();
}

}
