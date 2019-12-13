
#pragma once

#include <geometry/AABB.h>
#include <geometry/Ellipsoid.h>
#include <renderer/Color.h>
#include <renderer/VertexArray3D.h>
#include <memory>

namespace orf_n {
	class Program;
	class Texture2D;
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

	const HeightMap *getHeightMap() const;

	const orf_n::Texture2D *getPositionsHigh() const;

	const orf_n::Texture2D *getPositionsLow() const;

	const QuadTree *getQuadTree() const;

	// Returns the bounding box relative to heightmap in flat coords
	// @todo: this will have to give way to the cartesian bb
	const orf_n::AABB *getAABB() const;

	const omath::dvec2 getLatLon() const;

	/**
	 * Center of the tile in world cartesian coords
	 */
	const omath::dvec3 &getWorldCenter() const;

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
	 * GL_RGB32F type textures holding world space cartesian coordinates of the heightmap texels.
	 * One has the high, the other the low part. Texture units are 1 and 2.
	 */
	std::unique_ptr<orf_n::Texture2D> m_positionsHigh{nullptr};

	std::unique_ptr<orf_n::Texture2D> m_positionsLow{nullptr};

	void buildPositionTextures( const orf_n::Ellipsoid *const ellipsoid );

	/**
	 * Bounding box relative to tile
	 */
	std::unique_ptr<orf_n::AABB> m_AABB{nullptr};

	/**
	 * Lower left longitude and latitude, angle between two posts in arcsec.
	 * Used to build the position texture.
	 */
	double m_lon{0.0};

	double m_lat{0.0};

	double m_cellSize{0.0};

	omath::dvec3 m_worldCenter;

};

}
