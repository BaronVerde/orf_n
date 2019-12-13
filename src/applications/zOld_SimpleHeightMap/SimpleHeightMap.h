
#pragma once

#include <renderer/Program.h>
#include <renderer/Texture2D.h>
#include <renderer/TextureRectangle.h>
#include <scene/Renderable.h>

#include "Terrain/FlatMesh.h"
#include "Terrain/HeightMap.h"

class SimpleHeightMap : public orf_n::Renderable {
public:

    /**
     * @param filename Filename of the height data, 16bit uint, 1024*1024.
     */
	SimpleHeightMap( const std::string &filename );

	virtual ~SimpleHeightMap();

	void setup() override final;

	void render() override final;

	void cleanup() override final;

private:
	std::string m_filename;

	orf_n::Program *m_simpleTerrainShader{ nullptr };

	orf_n::Program *m_normalShader{ nullptr };

	bool m_showNormals{ false };

	int m_normalAlgorithm{ 0 };

	int m_shadingAlgorithm{ 0 };

	float m_heightExaggeration{ 5.0f };

	float m_sobelFactor{ 1.0f };

	/**
	 * @todo: why is y negative ?
	 */
	glm::vec3 m_sunPosition{ 0.0f, 250.0f, 0.0f };

	terrain::HeightMap *m_heightMap{ nullptr };

	terrain::FlatMesh *m_flatMesh{ nullptr };

	orf_n::Texture2D *m_textureColorMap{ nullptr };

	orf_n::Texture2D *m_textureColorRampHeight{ nullptr };

	orf_n::Texture2D *m_textureColorRampSlope{ nullptr };

	orf_n::Texture2D *m_textureColorRampGrassStone{ nullptr };

	orf_n::Texture2D *m_textureGrass{ nullptr };

	orf_n::Texture2D *m_textureStone{ nullptr };

	orf_n::Texture2D *m_textureGrassStoneBlendMap{ nullptr };

};
