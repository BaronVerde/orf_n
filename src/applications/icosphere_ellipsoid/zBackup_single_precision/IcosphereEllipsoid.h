
/**
 * Create an ellipsoid mesh by subdivision.
 * Intakes the 3 axes of an ellipsoid and the number of subdivisions.
 *
 * @todo: parametrize textures.
 */

#pragma once

#include <base/GlfwWindow.h>
#include <geometry/Icosphere.h>
#include <noise/MapBuilder.h>
#include <omath/vec3.h>
#include <renderer/IndexBuffer.h>
#include <renderer/Program.h>
#include <renderer/Texture2D.h>
#include <renderer/VertexArray3D.h>
#include <scene/Renderable.h>

class IcosphereEllipsoid : public orf_n::Renderable {
public:
	IcosphereEllipsoid( orf_n::GlfwWindow *win,
						const omath::vec3 &axes,
						const uint32_t numSubDivs );

	virtual ~IcosphereEllipsoid();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:
	orf_n::Icosphere *m_ico{ nullptr };

	orf_n::Program *m_shader{ nullptr };

	orf_n::IndexBuffer *m_indexBuffer{ nullptr };

	/**
	 * High and low parts interleaved
	 */
	orf_n::VertexArray3D<omath::vec3> *m_vertexArray{ nullptr };

	orf_n::Texture2D *m_texture{ nullptr };

	omath::vec3 m_axes{ 1.0, 1.0, 1.0 };

	uint32_t m_numSubDivs{ 1 };

	bool m_visible{ true };

	bool m_perlinActive{ true };

	bool m_billowsActive{ true };

	bool m_ridgesActive{ false };

	bool m_selectorActive{ false };

	bool m_showDisplacement{ true };

	double m_perlinFrequency{ noise::DEFAULT_FREQUENCY };

	double m_perlinLacunarity{ noise::DEFAULT_LACUNARITY };

	double m_perlinPersistence{ noise::DEFAULT_PERSISTENCE };

	int m_perlinOctaveCount{ noise::DEFAULT_OCTAVE_COUNT };

	omath::ivec2 m_mapDimension{ 2048, 1024 };

	double m_billowsFrequency{ noise::DEFAULT_FREQUENCY };

	double m_billowsLacunarity{ noise::DEFAULT_LACUNARITY };

	double m_billowsPersistence{ noise::DEFAULT_PERSISTENCE };

	int m_billowsOctaveCount{ noise::DEFAULT_OCTAVE_COUNT };

	double m_ridgesFrequency{ noise::DEFAULT_FREQUENCY };

	double m_ridgesLacunarity{ noise::DEFAULT_LACUNARITY };

	//double m_ridgesPersistence{ noise::DEFAULT_PERSISTENCE };

	int m_ridgesOctaveCount{ noise::DEFAULT_OCTAVE_COUNT };

	double m_lowerBound{ 0.0 };

	double m_upperBound{ 1000.0 };

	double m_falloff{ 0.125 };

	// 0 = cenrtic, 1 = detic, 2 = combined
	int m_normalType{ 2 };

};
