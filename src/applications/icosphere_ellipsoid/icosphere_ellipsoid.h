
/**
 * Create an ellipsoid mesh by subdivision.
 * Intakes the 3 axes of an ellipsoid and the number of subdivisions.
 *
 * @todo: parametrize textures.
 */

#pragma once

#include <renderer/program.h>
#include "geometry/geodetic.h"
#include "geometry/icosphere.h"
#include "renderer/texture_2d.h"
#include "scene/renderable.h"
#include "geometry/aabb.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexArray3D.h"
//#include "Noise/MapBuilder.h"

class icosphere_ellipsoid : public orf_n::renderable {
public:
	icosphere_ellipsoid( const omath::vec3 &axes, const uint32_t numSubDivs );

	virtual ~icosphere_ellipsoid();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:
	orf_n::icosphere *m_ico{ nullptr };

	orf_n::program *m_shader{ nullptr };

	orf_n::IndexBuffer *m_indexBuffer{ nullptr };

	omath::dmat4 m_modelMatrix{ 1.0 };

	omath::dmat4 m_modelViewMatrix{ 1.0 };

	/**
	 * High and low parts interleaved
	 */
	orf_n::VertexArray3D<omath::dvec3> *m_vertexArray{ nullptr };

	orf_n::VertexArray3D<omath::dvec3> *m_boxArray{ nullptr };

	orf_n::VertexArray3D<omath::dvec3> *m_tileBorderArray{ nullptr };

	orf_n::IndexBuffer *m_tileBorderIndices{ nullptr };

	orf_n::texture_2d *m_texture{ nullptr };

	omath::dvec3 m_axes{ 1.0, 1.0, 1.0 };

	std::vector<orf_n::aabb> m_relativeBoxes;

	std::vector<orf_n::geodetic> m_geodeticCoords;

	double m_cellsize{0.0};

	uint32_t m_numSubDivs{ 1 };

	bool m_visible{ true };

	// 0 = cenrtic, 1 = detic, 2 = combined
	int m_normalType{ 2 };

	void readBB( const std::string &filename );

};
