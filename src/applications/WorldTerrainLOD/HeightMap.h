
#pragma once

#include <geometry/Rectangle.h>
#include <omath/vec2.h>
#include <renderer/Texture2D.h>
#include <string>

namespace terrain {

/**
 * A 2D heightmap texture for use as displacement on a FlatMesh.
 * Stores height values for lookup.
 * @todo check if rectangular
 * Attention: Heightmap x and y are heightmap horizontal coordinates, the height is the value.
 * In world space (and gridmesh as well as wuad tree and nodes),
 * horizontal ccords are x and z, the height value is y.
 */
class HeightMap {
public:

	static constexpr GLuint HEIGHTMAP_TEXTURE_UNIT{0};

	typedef enum : unsigned int {
		B8, B16
	} bitDepth_t;

	/**
	 * Create Heightmap object with VertexArray and IndexBuffer from file.
	 * Apply an factor to the height values, usually to have smaller numbers.
	 */
	HeightMap( const std::string &filename, const bitDepth_t depth = B16 );

	virtual ~HeightMap();

	void bind() const;

	void unbind() const;

	const omath::ivec2 &getExtent() const;

	const GLuint &getTexture() const;

	/**
	 * Returns min/max values in the world range of 0.0f..65535.0f
	 */
	omath::vec2 getMinMaxHeightArea( const int x, const int z, const int w, const int h ) const;

	/**
	 * Returns the real world height normalized value at coords
	 */
	float getHeightAt( const int x, const int y ) const;

	const omath::vec2 &getMinMaxHeight() const;

private:
	std::string m_filename{ "" };

	/**
	 * Real world height values, not normalized, normalized to 0..1 over the range of uint16_t
	 */
	float *m_heightValuesNormalized{ nullptr };

	GLuint m_texture{ 0 };

	/**
	 * Height/width of texture file in pixels.
	 * Integer because opengl expects integer in texture addressing and for loops compare to <=0 ...
	 */
	omath::ivec2 m_extent{ 0, 0 };

	/**
	 * Depth of the texture, factors are derivee from that.
	 */
	bitDepth_t m_bitDepth{ B16 };

	/**
	 * Minimum and maximum height values (int int16_t range) of the texture.
	 */
	omath::vec2 m_minMaxHeightValues{ 0.0f, 65535.0f };

	const bitDepth_t &getDepth() const;

};

}
