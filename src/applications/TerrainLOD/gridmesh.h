
/**
 * A rectangular, [0.0..1.0] clamped regular flat mesh.
 * X and Z are the horizontal dimensions. Y will be extruded by the heightmap.
 */

#pragma once

#include "glad/glad.h"

namespace terrain {

class gridmesh {
public:
	gridmesh( int dimension );

	virtual~gridmesh();

	int getDimension() const;

	int getEndIndexTL() const;

	int getEndIndexTR() const;

	int getEndIndexBL() const;

	int getEndIndexBR() const;

	int getNumberOfSubMeshIndices() const;

	GLsizei get_number_indices() const;

	void bind() const;

	void unbind() const;

private:
	const GLuint VERTEX_BUFFER_BINDING_INDEX{0};

	GLuint m_vertexArray;

	GLuint m_indexBuffer;

	GLuint m_vertexBuffer;

	int m_dimension{ 0 };

	int m_endIndexTopLeft{ 0 };

	int m_endIndexTopRight{ 0 };

	int m_endIndexBottomLeft{ 0 };

	int m_endIndexBottomRight{ 0 };

	int m_numberOfSubmeshIndices{ 0 };

	GLsizei m_number_of_indices{0};

};

}
