
#pragma once

#include <applications/terrain_lod/settings.h>
#include <scene/renderable.h>
#include "renderer/Color.h"
#include "renderer/DrawPrimitives.h"
#include "renderer/VertexArray3D.h"

namespace terrain {
	class TerrainTile;
	class gridmesh;
	class LODSelection;
}

namespace orf_n {
	class program;
	class IndexBuffer;
}

class TerrainLOD : public orf_n::renderable {
public:

	TerrainLOD();

	virtual ~TerrainLOD();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:
	const std::vector<std::string> TERRAIN_FILES {
		"resources/textures/terrain/area_52_06/tile_2048_1",
		/*"resources/textures/Terrain/Area_52_06/tile_2048_2",
		"resources/textures/Terrain/Area_52_06/tile_2048_3",
		"resources/textures/Terrain/Area_52_06/tile_2048_4"*/
	};

	// Maximum number of tiles simultaneously loaded in memory.
	const int MAX_NUMBER_OF_TILES{ (int)TERRAIN_FILES.size() };

	struct renderStats_t {
		int totalRenderedNodes{ 0 };
		int totalRenderedTriangles{ 0 };
		void reset() {
			totalRenderedTriangles = totalRenderedNodes = 0;
		}
	} m_renderStats;

	std::vector<terrain::TerrainTile *> m_terrainTiles;

	std::unique_ptr<terrain::gridmesh> m_drawGridMesh{ nullptr };

	std::unique_ptr<orf_n::program> m_shaderTerrain{ nullptr };

	/**
	 * Selection object. Used to store selected nodes for rendering every frame.
	 */
	terrain::LODSelection *m_lodSelection{ nullptr };

	/**
	 * These figures are identical for all tiles of the same size. They hold the texture sizes
	 * and their ratio tile to texture. This implies that all tiles must have equal size
	 * and all textures equal resolution. Otherwise these values would have to be tile specific.
	 */
	omath::vec2 m_tileToTexture;

	omath::vec4 m_heightMapInfo;

	// Lighting @todo, and it is the direction, not the position.
	omath::vec3 m_diffuseLightPos{ -5.0f, -1.0f, 0.0f };

	omath::mat4 m_modelMatrix{ 1.0f };

	// Returns true when shader uniforms need to be updated.
	bool refreshUI();

	/**
	 * **** Debug stuff ****
	 */
	// Fast access is desirable. It's slow anyway, immediate as it is now
	orf_n::DrawPrimitives &m_drawPrimitives{ orf_n::DrawPrimitives::getInstance() };

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
