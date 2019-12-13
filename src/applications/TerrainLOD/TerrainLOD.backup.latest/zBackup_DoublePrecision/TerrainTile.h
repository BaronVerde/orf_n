
#pragma once

#include <geometry/AABB.h>
#include <renderer/Color.h>

namespace orf_n {
	class Program;
}

namespace terrain {

class HeightMap;
class GridMesh;
class QuadTree;

class TerrainTile {
public:

	TerrainTile( const std::string &filename );

	TerrainTile( const TerrainTile &other ) = delete;

	TerrainTile &operator=( const TerrainTile &other ) = delete;

	TerrainTile( TerrainTile &&other ) = delete;

	TerrainTile &operator=( TerrainTile &&other ) = delete;

	virtual ~TerrainTile();

	const terrain::HeightMap *getHeightMap() const;

	const terrain::QuadTree *getQuadTree() const;

	// Returns the boudning box relative to heightmap in flat coords
	// @todo: this will have to give way to the cartesian bb
	const orf_n::dAABB *getAABB() const;

	const omath::dvec2 getLonLat() const;

	const double &getCellSize() const;

	/**
	 * Needs a pointer to the shader because of level's morph consts and node's offsets and sizes
	 * Returns number of rendered nodes (x) and triangles (y)
	 */
	omath::uvec2 render( const orf_n::Program *const p,
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

	/**
	 * Bounding box relative to tile
	 */
	const orf_n::dAABB *m_AABB{ nullptr };

	/**
	 * Lower left longitude and latitude, angle between two posts in arcsec
	 */
	double m_lon{0.0};

	double m_lat{0.0};

	double m_cellSize{0.0};

};

}
