
#pragma once

#include <applications/TerrainLOD/GridMesh.h>
#include <applications/TerrainLOD/TerrainTile.h>
#include <geometry/Ellipsoid.h>
#include <renderer/DrawPrimitives.h>
#include <renderer/Program.h>
#include <scene/Renderable.h>

class Positions : public orf_n::Renderable {
public:
	Positions();

	virtual ~Positions();

	virtual void setup() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:
	// Maximum number of tiles simultaneously loaded in memory.
	static const int MAX_NUMBER_OF_TILES{ 1 };

	const std::vector<std::string> TERRAIN_FILES {
		"Resources/Textures/Terrain/Area_52_06/tile_2048_1",
		"Resources/Textures/Terrain/Area_52_06/tile_2048_2",
		"Resources/Textures/Terrain/Area_52_06/tile_2048_3",
		"Resources/Textures/Terrain/Area_52_06/tile_2048_4"
	};

	std::unique_ptr<orf_n::Program> m_program{nullptr};

	std::unique_ptr<terrain::GridMesh> m_gridMesh{nullptr};

	std::unique_ptr<orf_n::Ellipsoid> m_ellipsoid{nullptr};

	std::unique_ptr<terrain::TerrainTile> m_terrainTile{nullptr};

	orf_n::DrawPrimitives &m_drawPrimitives{ orf_n::DrawPrimitives::getInstance() };

};
