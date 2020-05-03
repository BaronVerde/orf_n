
/**
 * An ellipsoid cube map as a base to build a tile database for planetary data.
 * Each of the vertices represents the posiion of a terrain tile on the ellipsoid.
 */

#pragma once

#include <geometry/ellipsoid.h>

namespace orf_n {

class EllipsoidCM : public ellipsoid {
public:
	/**
	 * Axes; .x are the semi major axes in cartesian x and y (equatorial) plane,
	 * .y is semi minor axis representing z (though the poles).
	 * Number of tiles per cube map face. Must be power of 2. There will be 1 vertex more.
	 * For example 64 tiles per face for an earth sized planet when tile size is 2048 posts,
	 * will result in ~76.29m resolution on a great circle on earth or 20.83m on the moon.
	 * That would be 0.000686646° between posts, compared to 0.000833 of srtm data.
	 */
	EllipsoidCM( const omath::dvec2 axes, const unsigned int numberOfTilesPerFace );

	virtual ~EllipsoidCM();

private:
	unsigned int m_numSubdivsPerFace;

	/**
	 * Holds vertices face by face in the order of side_t
	 */
	std::vector<omath::dvec3> m_vertices;

	// Helper func
	void simpleCubeToEllipsoid();

};

} /* namespace orf_n */
