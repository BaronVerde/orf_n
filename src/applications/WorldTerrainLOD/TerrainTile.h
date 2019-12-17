
#pragma once

#include "geometry/AABB.h"
#include "renderer/Color.h"
#include "renderer/VertexArray3D.h"
#include <memory>

namespace orf_n {
	class Program;
	class Texture2D;
	class Ellipsoid;
}

namespace terrain {

class HeightMap;
class GridMesh;
class QuadTree;
class LODSelection;

class TerrainTile {
public:

	/**
	 * Pathname of the tile heightmap and reference ellipsoid.
	 * Ellispoid is used to calculate world cartesian positions of posts from lower left corner
	 * and anular distance between posts. Positions are stored as high/low floats in two textures.
	 */
	TerrainTile( const std::string &filename, const orf_n::Ellipsoid *const ellipsoid );

	TerrainTile( const TerrainTile &other ) = delete;

	TerrainTile &operator=( const TerrainTile &other ) = delete;

	TerrainTile( TerrainTile &&other ) = default;

	TerrainTile &operator=( TerrainTile &&other ) = default;

	virtual ~TerrainTile();

	const QuadTree *getQuadTree() const;

	const HeightMap *getHeightMap() const;

	// Returns the bounding box relative to heightmap in flat coords
	// @todo: this will have to give way to the cartesian bb
	const orf_n::AABB *getAABB() const;

	const omath::dvec2 getLatLon() const;

	const orf_n::Ellipsoid *getEllipsoid() const;

	const double &getCellSize() const;

	/**
	 * Needs a pointer to the shader to pass in uniforms
	 * Returns number of rendered nodes (x) and triangles (y)
	 */
	omath::uvec2 render( const orf_n::Program *const p,
						 const terrain::GridMesh *const gridMesh,
						 const terrain::LODSelection *const selection,
						 const int tileIndex,
						 const GLint drawMode );

private:

	const std::string m_filename{""};

	/**
	 * @todo Heightmap texture unit is 0 hard coded. A resource manager will have to take care in the future.
	 */
	std::unique_ptr<HeightMap> m_heightMap{nullptr};

	std::unique_ptr<QuadTree> m_quadTree{nullptr};

	/**
	 * Bounding box in world coordinates
	 */
	std::unique_ptr<orf_n::AABB> m_AABB{nullptr};

	/**
	 * Lower left longitude and latitude, angle between two posts in arcsec.
	 * Used to build the position texture.
	 */
	double m_lon{0.0};

	double m_lat{0.0};

	double m_cellsize{0.0};

	/**
	 * Reference ellipsoid pointer stored here, needed to calculate cartesian positions
	 */
	const orf_n::Ellipsoid *m_ellipsoid{nullptr};

};

}
