
/**
 * Singleton class to draw primitives.
 * Only meant for debug or tests so far because bad performance.
 * Immediate drawing of a sphere, a box
 * @todo: this is not thread safe. setup and cleanup should be mutex protected or setup globally !
 */
#pragma once

#include "geometry/AABB.h"
#include "Color.h"
#include "IndexBuffer.h"
#include "Program.h"
#include "VertexArray3D.h"

namespace orf_n {

class OBB;
class Ellipsoid;

class DrawPrimitives {
public:

	static DrawPrimitives &getInstance();

	/**
	 * Sets up the vertex arrays and index buffers for drawing
	 */
	void setupDebugDrawing();

	/**
	 * @todo: issue a warning if this was not called, because program crashes when destructor calls it.
	 */
	void cleanupDebugDrawing();

	void drawAABB( const AABB &bb, const color_t &color ) const;

	void drawOBB( const OBB &bb, const color_t &color ) const;

	void drawEllipsoid( const Ellipsoid &ellipsoid, const color_t &color ) const;

	void drawSphere( const omath::vec3 &position, const float &radius, const color_t &color ) const;

	const Program *getProgramPtr() const;

	const VertexArray3D<omath::dvec3> *getBoxArray() const;

	// Array contents can be changed, the pointer not
	const VertexArray3D<omath::dvec3> *getLineArray() const;

	const IndexBuffer *getBoxIndices() const;

private:
	DrawPrimitives() = default;

	DrawPrimitives( const DrawPrimitives & ) = delete;

	void operator=( const DrawPrimitives & ) = delete;

	virtual ~DrawPrimitives();

	std::unique_ptr<Program> m_shaderDebug{ nullptr };

	std::unique_ptr<VertexArray3D<omath::dvec3>> m_boxArray{ nullptr };

	std::unique_ptr<IndexBuffer> m_boxIndices{ nullptr };

	std::unique_ptr<VertexArray3D<omath::dvec3>> m_sphereArray{ nullptr };

	std::unique_ptr<IndexBuffer> m_sphereIndices{ nullptr };

	std::unique_ptr<VertexArray3D<omath::dvec3>> m_lineArray{ nullptr };

	bool m_allSetup{ false };

};

}
