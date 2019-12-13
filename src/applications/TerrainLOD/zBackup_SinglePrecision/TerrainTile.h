
#pragma once

#include <renderer/Color.h>

namespace terrain {
	class HeightMap;
	class GridMesh;
	class QuadTree;
}

namespace orf_n {
	class Program;
	class AABB;
}

namespace terrain {

class TerrainTile {
public:
	explicit TerrainTile( const std::string &filename );

	TerrainTile( const TerrainTile &other ) = delete;

	TerrainTile &operator=( const TerrainTile &other ) = delete;

	TerrainTile( TerrainTile &&other ) noexcept = default;

	TerrainTile &operator=( TerrainTile &&other ) noexcept = default;

	virtual ~TerrainTile();

	const terrain::HeightMap *getHeightMap() const;

	const terrain::QuadTree *getQuadTree() const;

	const orf_n::AABB *getTileBoundingBox() const;

	void render( const orf_n::Program *const p,
				 const terrain::GridMesh *const gridMesh,
				 const terrain::LODSelection *const selection,
				 const int tileIndex,
				 const GLint drawMode );

private:
	// Bind heightmap texture to shader unit 0. @todo Are there conflicts ?
	const GLuint HEIGHTMAP_UNIT{ 0 };

	const std::string m_filename{ "" };

	const terrain::HeightMap *m_heightMap{ nullptr };

	const terrain::QuadTree *m_quadTree{ nullptr };

	// These figures are tile specific as they hold the world coordinates
	// @todo: eventually use the heightmap's bounding box.
	omath::vec2 m_quadWorldMax;

	omath::vec3 m_terrainScale;

	omath::vec3 m_terrainOffset;

};

}
