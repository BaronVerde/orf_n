
// after: http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

#pragma once

#include <GL/glew.h>
#include <omath/vec3.h>
#include <vector>
#include <map>

namespace orf_n {

class Icosphere {
public:
	Icosphere( const omath::vec3 &radii, const uint32_t &numSubDivs );

	virtual ~Icosphere();

	const std::vector<omath::vec3> &getVertices() const;

	const std::vector<GLuint> &getIndices() const;

	const omath::vec3 &getRadii() const;

	const omath::vec3 &getRadiiSquared() const;

	const omath::vec3 &getOneOverRadiiSquared() const;
	
	static omath::vec3 centricSurfaceNormal( const omath::dvec3 &positionOnEllipsoid );
	
	omath::vec3 geodeticSurfaceNormal( const omath::dvec3 &positionOnEllipsoid ) const;

private:
	omath::vec3 m_radii{ 1.0f, 1.0f, 1.0f };

	uint32_t m_numSubDivs;

	std::vector<omath::vec3> m_positions;

	std::vector<GLuint> m_indices;

	GLuint m_index{0};

	// @todo: An icosphere should carry an ellipsoid object to hold these values
	omath::vec3 m_radiiSquared;

	omath::vec3 m_radiiToTheFourth;

	omath::vec3 m_oneOverRadiiSquared;

	/**
	 * For each iteration, temporarily store the indices with a key to avoid
	 * duplicate indices when subdividing.
	 */
    std::map<uint64_t, GLuint> m_middlePointIndexCache;

	GLuint addVertex( const omath::vec3 &p );

	GLuint getMiddlePoint( const GLuint &p1, const GLuint &p2 );

};

} /* namespace orf_n */
