
#include <base/Logbook.h>
#include <geometry/krams/Icosphere.h>

namespace orf_n {

Icosphere::Icosphere( const omath::vec3 &radii, const uint32_t &numSubDivs ) :
		m_radii{ radii }, m_numSubDivs{ numSubDivs }, m_index{0}, m_radiiSquared{ m_radii * m_radii },
		m_radiiToTheFourth{ m_radiiSquared * m_radiiSquared }, m_oneOverRadiiSquared{ 1.0f / m_radiiSquared } {
	// Create initial icosahedron, 12 vertices
	double t{ ( 1.0 + std::sqrt( 5.0 ) ) / 2.0 };
	addVertex( omath::vec3{ -1,  t,  0 } );
	addVertex( omath::vec3{  1,  t,  0 } );
	addVertex( omath::vec3{ -1, -t,  0 } );
	addVertex( omath::vec3{  1, -t,  0 } );
	addVertex( omath::vec3{  0, -1,  t } );
	addVertex( omath::vec3{  0,  1,  t } );
	addVertex( omath::vec3{  0, -1, -t } );
	addVertex( omath::vec3{  0,  1, -t } );
	addVertex( omath::vec3{  t,  0, -1 } );
	addVertex( omath::vec3{  t,  0,  1 } );
	addVertex( omath::vec3{ -t,  0, -1 } );
	addVertex( omath::vec3{ -t,  0,  1 } );

	// Create 20 triangles of the icosahedron
	m_indices = {
		// 5 faces around point 0
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		// 5 adjacent faces
		1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
		// 5 faces around point 3
		3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		// 5 adjacent faces
		4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
	};

	// refine triangles
	for( uint32_t i{0}; i < m_numSubDivs; ++i ) {
		m_middlePointIndexCache.clear();
		std::vector<GLuint> tmpIndices;
		for( size_t j{0}; j < m_indices.size(); j += 3 ) {
			// replace triangle by 4 triangles
			GLuint a{ getMiddlePoint( m_indices[j], m_indices[j+1] ) };
			GLuint b{ getMiddlePoint( m_indices[j+1], m_indices[j+2] ) };
			GLuint c{ getMiddlePoint( m_indices[j+2], m_indices[j] ) };
			tmpIndices.push_back( m_indices[j] );
			tmpIndices.push_back( a );
			tmpIndices.push_back( c );
			tmpIndices.push_back( m_indices[j+1] );
			tmpIndices.push_back( b );
			tmpIndices.push_back( a );
			tmpIndices.push_back( m_indices[j+2] );
			tmpIndices.push_back( c );
			tmpIndices.push_back( b );
			tmpIndices.push_back( a );
			tmpIndices.push_back( b );
			tmpIndices.push_back( c );
		}
		m_indices.clear();
		m_indices = tmpIndices;
	}

	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, "Icosphere created." );

	// scale to ellipsoid radii
	for( omath::vec3 &v : m_positions )
		v *= m_radii;

}

Icosphere::~Icosphere() {
	orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::INFO, "Icosphere deleted." );
}

// add vertex to mesh, fix position to be on unit sphere, return index
GLuint Icosphere::addVertex( const omath::vec3 &p ) {
	 m_positions.push_back( omath::normalize( p ) );
     return m_index++;
}

// Return index of point in the middle of p1 and p2 and if not existent
// add it to the vector of positions
GLuint Icosphere::getMiddlePoint( const GLuint &p1, const GLuint &p2 ) {
	// first check if we have it already
	uint64_t smallerIndex{ std::min( p1, p2 ) };
	uint64_t greaterIndex{ std::max( p1, p2 ) };
	uint64_t indexKey{ ( smallerIndex << 32 ) + greaterIndex };
	// If already there, then return that index
	const std::map<uint64_t, GLuint>::iterator retVal{ m_middlePointIndexCache.find( indexKey ) };
	if( retVal != m_middlePointIndexCache.end() ) {
		GLuint returnIndex{ retVal->second };
		return returnIndex;
	}
	// Not in cache ? Calculate its position, add it and increment index counter
	omath::vec3 point1{ m_positions[p1] };
	omath::vec3 point2{ m_positions[p2] };
	omath::vec3 middle{ omath::vec3{ ( point1 + point2 ) / 2.0f } };
	// add vertex makes sure point is on unit sphere
	GLuint newIndex{ addVertex( middle ) };
	// store it, return index
	m_middlePointIndexCache.insert( std::make_pair( indexKey, newIndex ) );
	return newIndex;
}

const std::vector<omath::vec3> &Icosphere::getVertices() const {
	return m_positions;
}

const std::vector<GLuint> &Icosphere::getIndices() const {
	return m_indices;
}

const omath::vec3 &Icosphere::getRadii() const {
	return m_radii;
}

const omath::vec3 &Icosphere::getRadiiSquared() const {
	return m_radiiSquared;
}

const omath::vec3 &Icosphere::getOneOverRadiiSquared() const {
	return m_oneOverRadiiSquared;
}

// static
omath::vec3 Icosphere::centricSurfaceNormal( const omath::dvec3 &pos ) {
	return omath::normalize( pos );
}

omath::vec3 Icosphere::geodeticSurfaceNormal( const omath::dvec3 &pos ) const {
	return omath::normalize( omath::vec3{ pos } * m_oneOverRadiiSquared );
}

} /* namespace orf_n */
