
#pragma once

#include <applications/TerrainLOD/TerrainLOD.backup.latest/zBackup_SinglePrecision/Settings.h>
#include <renderer/Color.h>
#include <renderer/DrawPrimitives.h>
#include <renderer/VertexArray3D.h>
#include <scene/Renderable.h>
#include <memory>

namespace terrain {
	class TerrainTile;
	class GridMesh;
	class LODSelection;
}

namespace orf_n {
	class Program;
	class IndexBuffer;
	class TerrainCamera;
}

class StreamingTerrain : public orf_n::Renderable {
public:
	StreamingTerrain();

	virtual ~StreamingTerrain();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:
	const std::vector<std::string> TERRAIN_FILES {
		"Resources/Textures/Terrain/tile_2048_1",
		"Resources/Textures/Terrain/tile_2048_2",
		"Resources/Textures/Terrain/tile_2048_3",
		"Resources/Textures/Terrain/tile_2048_4"
	};

	struct renderStats_t {
		int totalRenderedQuads{ 0 };
		int totalRenderedTriangles{ 0 };
		void reset() {
			totalRenderedTriangles = totalRenderedQuads = 0;
		}
	} m_renderStats;

	std::vector<terrain::TerrainTile *> m_terrainTiles;

	const terrain::GridMesh *m_drawGridMesh{ nullptr };

	const orf_n::Program *m_shaderTerrain{ nullptr };

	/**
	 * Selection object. Used to store selected nodes for rendering every frame.
	 * Also olds level ranges.
	 */
	terrain::LODSelection *m_lodSelection{ nullptr };

	// These figures are identical for all tiles of the same size.
	omath::vec2 m_worldToTexture;

	omath::vec4 m_heightMapInfo;

	// Lighting @todo, and it is the direction, not the position.
	omath::vec3 m_diffuseLightPos{ -5.0f, -1.0f, 0.0f };

	// Returns true when shader uniforms need to be updated.
	bool refreshUI();

	/**
	 * **** Debug stuff ****
	 */
	// Fast access is desirable. It's slow anyway, immediate as it is now
	const orf_n::DrawPrimitives &m_drawPrimitives{ orf_n::DrawPrimitives::getInstance() };

	void debugDrawing();

	void debugDrawLowestLevelBoxes( const terrain::TerrainTile *const t ) const;

	// Whether to show the boxes of the terrain tiles
	bool m_showTileBoxes{ false };

	// Show all boxes of the lowest level that are in the view frustum. Can be many and very slow.
	bool m_showLowestLevelBoxes{ false };

	// Show boxes of all selected nodes (immediate mode, low)
	bool m_showSelectedBoxes{ false };

	// Actually draw the terrain of the selected nodes.
	bool m_drawSelection{ true };

};
