
// after: http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
/**
 * A mesh for an ellipsoid.
 */

#pragma once

#include "ellipsoid.h"
#include "omath/vec3.h"
#include <vector>
#include <map>
#include "glad/glad.h"

namespace orf_n {

class icosphere : public ellipsoid {
public:
	icosphere( const omath::dvec3 &radii, const uint32_t &num_sub_divs );

	virtual ~icosphere();

	const std::vector<omath::dvec3> &get_vertices() const;

	const std::vector<GLuint> &get_indices() const;

private:
	uint32_t m_num_sub_divs;

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
    std::map<uint64_t, GLuint> m_middle_point_index_cache;

    /**
     * Helper funcs and counter
     */
    GLuint m_index{0};

	GLuint add_vertex( const omath::dvec3 &p );

	GLuint get_middle_point( const GLuint &p1, const GLuint &p2 );

};

} // namespace
