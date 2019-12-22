
#include "HydroTile.h"
#include <iostream>
#include "omath/common.h"
#include <png++/image.hpp>
#include <png++/gray_pixel.hpp>

HydroTile::HydroTile() : orf_n::Renderable{ "HydroTile" } {
	if( m_extent == 0 || m_extent > 4096 || !omath::isPowerOf2( m_extent ) )
		std::cerr << "Extent must be > 0 and <= 4096 !" << std::endl;
	m_gridMesh = std::make_unique<terrain::GridMesh>( m_extent );
	// Creating the liftrate map will be done elsewhere later
	if( !omath::isPowerOf2( m_extent ) || m_extent < 256 || m_extent > 4096 )
		std::cout << "Extent of the patch must be power of 2 and between 256 and 4096" << std::endl;
	size_t numPoints{ m_profileEW.size() };
	if( numPoints >= m_extent || !omath::isPowerOf2( numPoints ) || numPoints != m_profileNS.size() )
		std::cerr << "Lists of control points must be equal in size, power of 2 and smaller than the extent" << std::endl;
	// create profile texture for lift rate
	png::image<png::gray_pixel> image( static_cast<uint32_t>(numPoints), static_cast<uint32_t>(numPoints) );
	std::ofstream o{ "testfile.txt", std::ios_base::out };
	for( size_t i{0}; i < numPoints; ++i ) {
		for( size_t j{0}; j < numPoints; ++j ) {
			const float valuef{ omath::lerp( m_profileNS[i], m_profileEW[j], 0.5f ) };
			const uint8_t value{ static_cast<uint8_t>( std::round( valuef * 100.0f ) ) };
			o << i << '\t' << j << '\t' << m_profileEW[i] << '\t' << m_profileNS[j] << '\t' << (unsigned int)value << std::endl;
			image[i][j] = png::gray_pixel( value );
		}
	}
	o.close();
	image.write( m_liftrateFilename );
}

HydroTile::~HydroTile() {}

void HydroTile::setup() {
	// load the liftrate texture
	m_liftrateTexture = std::make_unique<orf_n::Texture2D>( m_liftrateFilename, 1 );
}

void HydroTile::render() {
	std::cout << "Rendering" << std::endl;

}

void HydroTile::cleanup() {}
