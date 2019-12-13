
#pragma once

#include <omath/vec3.h>
#include <renderer/IndexBuffer.h>
#include <renderer/Program.h>
#include <renderer/VertexArray3D.h>
#include <scene/Renderable.h>

class CubeToEllipsoid : public orf_n::Renderable {
public:
	CubeToEllipsoid();

	virtual ~CubeToEllipsoid();

	virtual void setup() override;

	virtual void render() override;

	virtual void cleanup() override;

private:
	orf_n::VertexArray3D<omath::dvec3> *m_vertexArray{ nullptr };

	//orf_n::IndexBuffer *m_indexBuffer{ nullptr };

	orf_n::Program *m_shader{nullptr};

};
