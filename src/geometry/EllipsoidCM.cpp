
#include <base/Logbook.h>
#include <geometry/EllipsoidCM.h>

namespace orf_n {

EllipsoidCM::EllipsoidCM( const omath::dvec2 axes, const unsigned int numberOfTilesPerFace ) :
			Ellipsoid{axes.x, axes.x, axes.y}, m_numSubdivsPerFace{numberOfTilesPerFace} {
	// @todo: between 2 and 512 ?
	if( !omath::isPowerOf2( m_numSubdivsPerFace ) ) {
		std::string s{ "CubeToEllipsoid: number of tiles per face must be power of two" };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::ERROR, s );
		throw std::runtime_error( "CubeToEllipsoid: number of tiles per face must be power of two." );
	}
	simpleCubeToEllipsoid();
}

EllipsoidCM::~EllipsoidCM() {}

void EllipsoidCM::simpleCubeToEllipsoid() {
	size_t numVerticesPerFace{ ( m_numSubdivsPerFace + 1 ) * ( m_numSubdivsPerFace + 1 ) };
	m_vertices.resize( 6 * numVerticesPerFace );
	size_t i{ 0 };
	const omath::dvec3 diameter{ 2.0 * m_radii };
	const double step{ 1.0 / m_numSubdivsPerFace };
	for( size_t y{0}; y <= m_numSubdivsPerFace; ++y ) {
		const double yOffset{ static_cast<double>(y) * step - 0.5 };
		for( size_t x{0}; x <= m_numSubdivsPerFace; ++x ) {
			const double xOffset{ static_cast<double>(x) * step - 0.5 };
			m_vertices[i] = omath::normalize( omath::dvec3{ xOffset, yOffset, -0.5 } ) * diameter;
			m_vertices[i + numVerticesPerFace] = omath::normalize( omath::dvec3{ -0.5, xOffset, yOffset } ) * diameter;
			m_vertices[i + 2*numVerticesPerFace] = omath::normalize( omath::dvec3{ 0.5, xOffset, yOffset } ) * diameter;
			m_vertices[i + 3*numVerticesPerFace] = omath::normalize( omath::dvec3{ xOffset, yOffset, 0.5 } ) * diameter;
			m_vertices[i + 4*numVerticesPerFace] = omath::normalize( omath::dvec3{ xOffset, 0.5, yOffset } ) * diameter;
			m_vertices[i + 5*numVerticesPerFace] = omath::normalize( omath::dvec3{ xOffset, -0.5, yOffset } ) * diameter;
			++i;
		}
	}
}

} /* namespace orf_n */
