
/**
 * A rectangular, [0.0..1.0] clamped regular flat mesh.
 * X and Z are the horizontal dimensions. Y will be extruded by the heightmap.
 */

#pragma once

#include "renderer/IndexBuffer.h"
#include "renderer/VertexArray3D.h"

namespace terrain {

class GridMesh {
public:
	GridMesh( int dimension );

	virtual~GridMesh();

	int getDimension() const;

	int getEndIndexTL() const;

	int getEndIndexTR() const;

	int getEndIndexBL() const;

	int getEndIndexBR() const;

	int getNumberOfSubMeshIndices() const;

	int getNumberOfIndices() const;

	void bind() const;

	void unbind() const;

private:
	std::unique_ptr<orf_n::VertexArray3D<omath::vec3>> m_vertexArray{ nullptr };

	std::unique_ptr<orf_n::IndexBuffer> m_indexBuffer{ nullptr };

	int m_dimension{ 0 };

	int m_endIndexTopLeft{ 0 };

	int m_endIndexTopRight{ 0 };

	int m_endIndexBottomLeft{ 0 };

	int m_endIndexBottomRight{ 0 };

	int m_numberOfSubmeshIndices{ 0 };

};

}
