
// after: http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
/**
 * A mesh for an ellipsoid.
 */

#pragma once

#include <geometry/Ellipsoid.h>
#include <omath/vec3.h>
#include <vector>
#include <map>
#include "../../extern/glad/glad.h"

namespace orf_n {

class Icosphere : public Ellipsoid {
public:
	Icosphere( const omath::dvec3 &radii, const uint32_t &numSubDivs );

	virtual ~Icosphere();

	const std::vector<omath::dvec3> &getVertices() const;

	const std::vector<GLuint> &getIndices() const;

private:
	uint32_t m_numSubDivs;

	/**
	 * Array of position vertices for the IcoSphere
	 */
	std::vector<omath::dvec3> m_positions;

	/**
	 * Indices for ccw drawing the positions
	 */
	std::vector<GLuint> m_indices;

	/**
	 * For each iteration, temporarily store the indices with a key to avoid
	 * duplicate indices when subdividing. Internal use.
	 */
    std::map<uint64_t, GLuint> m_middlePointIndexCache;

    /**
     * Helper funcs and counter
     */
    GLuint m_index{0};

	GLuint addVertex( const omath::dvec3 &p );

	GLuint getMiddlePoint( const GLuint &p1, const GLuint &p2 );

};

} // namespace
