
/**
 * Params:
 * name of asc file
 * tile size, default 2048
 * reference ellipsoid semi major axis, default WGS84
 * reference ellipsoid semi minor axis
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>		// strcmp
#include <cstdio>
#include <png++/image.hpp>
#include <png++/gray_pixel.hpp>
#include "Omath/vec3.h"
#include "Geometry/Geodetic.h"
#include "Geometry/Ellipsoid.h"

namespace converter {

// header info of an ascii srtm-90 file, the only input format
struct asciiFileHeader_t {
	uint32_t numberOfColumns;
	uint32_t numberOfRows;
	double llLong;
	double llLat;
	double cellsize;
	int noDataValue;
	uint16_t tilesize;
};

// header info for one output tile
struct tileHeader_t {
	uint16_t tileSize;		// size of the tile
	double llLatGeodetic;	// lower left post in geodetic coordinates
	double llLonGeodetic;
	double angularCellSize;	// angular distance between posts
	uint16_t minValue;		// minimum height value of this tile
	uint16_t maxValue;
};

bool readSrtmAsciiHeader( std::ifstream &inFile, asciiFileHeader_t &asciiFileHeader, const uint16_t tilesize ) {
	// Read header data, hader data of srtm v 4.1 files is fixed
	std::string inString;
	std::getline( inFile, inString );
	if( 1 == std::sscanf( inString.c_str(), "ncols %u\n", &asciiFileHeader.numberOfColumns ) )
		std::cout << "Number of columns: " << asciiFileHeader.numberOfColumns << std::endl;
	else {
		std::cerr << "Error reading ascii header number of columns\n";
		return false;
	}
	std::getline( inFile, inString );
	if( 1 == std::sscanf( inString.c_str(), "nrows %u\n", &asciiFileHeader.numberOfRows ) )
		std::cout << "Number of rows: " << asciiFileHeader.numberOfRows << std::endl;
	else {
		std::cerr << "Error reading ascii header number of rows\n";
		return false;
	}
	if( asciiFileHeader.numberOfColumns == 0 || asciiFileHeader.numberOfRows == 0 ||
		asciiFileHeader.numberOfColumns < tilesize || asciiFileHeader.numberOfRows < tilesize ) {
		std::cerr << "Error, size of data could not be determined or tile size > size of data\n";
		return false;
	}
	std::getline( inFile, inString );
	if( 1 == std::sscanf( inString.c_str(), "xllcorner %lf\n", &asciiFileHeader.llLong ) )
		std::cout << "Lower left longitude: " << asciiFileHeader.llLong << std::endl;
	else {
		std::cerr << "Error reading ascii header lower left x (longitude)\n";
		return false;
	}
	std::getline( inFile, inString );
	if( 1 == std::sscanf( inString.c_str(), "yllcorner %lf\n", &asciiFileHeader.llLat ) )
		std::cout << "Lower left latitude: " << asciiFileHeader.llLat << std::endl;
	else {
		std::cerr << "Error reading ascii header lower left y (latitude)\n";
		return false;
	}
	if( asciiFileHeader.llLong < -180.0 || asciiFileHeader.llLat < -90.0 ||
		asciiFileHeader.llLong > 180.0 || asciiFileHeader.llLat > 90.0 ) {
		std::cerr << "Error in latitude or longitude; out of bounds\n";
		return false;
	}
	std::getline( inFile, inString );
	if( 1 == std::sscanf( inString.c_str(), "cellsize %lf\n", &asciiFileHeader.cellsize ) )
		std::cout << "Distance between posts in arc seconds: " << asciiFileHeader.cellsize << '\n';
	else {
		std::cerr << "Cellsize could not be determined\n";
		return false;
	}
	std::getline( inFile, inString );
	if( 1 == std::sscanf( inString.c_str(), "NODATA_value %d\n", &asciiFileHeader.noDataValue ) )
		std::cout << "No data value: " << asciiFileHeader.noDataValue << '\n';
	else {
		std::cerr << "Error reading no data value\n";
		return false;
	}
	return true;
}

void deg2dms( const double dec, uint32_t &deg, uint32_t &min, uint32_t &sec ) {
	deg = static_cast<uint32_t>( std::floor( dec ) );
	double restMins{ dec - std::floor( dec ) };
	min = static_cast<uint32_t>( std::floor( restMins * 60.0 ) );
	double restSecs{ restMins - std::floor( restMins * 60.0 ) };
	sec = static_cast<uint32_t>( restSecs * 60.0 );
}

/* Parameters:
 * pathname of ascii file to import
 * size of tiles to extract, default is 2048. Must be power of 2
 * semi major = x/y equatorial plane, default WGS84
 * semi minor = z (rotation axis), default WGS84 */
static bool asc2png(
		const std::string &inFilename,
		const uint16_t tilesize = 2048,
		const double semiMajorAxis = 6378137.0,
		const double semiMinorAxis = 6356752.314245 ) {

	std::string outFilename;
	std::ofstream outFile;

	std::ifstream inFile{ inFilename, std::ios::in };
	if( !inFile.is_open() ) {
		std::cerr << "Converter: error opening ascii file '" << inFilename << "'\n";
		return false;
	}
	if( !omath::isPowerOf2( tilesize ) || tilesize < 256 || tilesize > 8192 ) {
		std::cerr << "Converter: Tilesize must be power of 2 and between 256 and 8192\n";
		return false;
	}
	if( semiMajorAxis <= 0.0 ) {
		std::cerr << "Converter: Semi major axis must be > 0.0\n";
		return false;
	}
	if( semiMinorAxis <= 0.0 || semiMinorAxis > semiMajorAxis ) {
		std::cerr << "Semi minor axis must be > 0.0 and <= semi major axis\n";
		return false;
	}
	std::cout << "Converter:\n";
	std::cout << "\tinput ascii file '" << inFilename << "'\n";
	std::cout << "\ttile size: " << tilesize << '\n';
	std::cout << "\tsemi major axis of reference ellipsoid: " << semiMajorAxis << '\n';
	std::cout << "\tsemi minor axis of reference ellipsoid: " << semiMinorAxis << '\n';

	asciiFileHeader_t asciiFileHeader;
	std::cout << "\nReading header of ascii file:\n";
	if( !readSrtmAsciiHeader(
			inFile,
			asciiFileHeader,
			tilesize ) ) {
		inFile.close();
		return false;
	}

	// Write bounding box in geodetic coordinates, convert to cartesian
	omath::dvec3 axes{ semiMajorAxis, semiMajorAxis, semiMinorAxis };
	orf_n::Ellipsoid eps{ axes };
	omath::dvec3 llCartesian{
		eps.toCartesian( orf_n::Geodetic{
			omath::radians( asciiFileHeader.llLong ),
			omath::radians( asciiFileHeader.llLat )
		} )
	};
	std::cout << "Lower left in cartesian coords: " << std::fixed << llCartesian << std::endl;

	const uint32_t numberOfHTiles{ static_cast<uint16_t>( std::floor( asciiFileHeader.numberOfColumns / tilesize ) ) };
	const uint32_t numberOfVTiles{ static_cast<uint16_t>( std::floor( asciiFileHeader.numberOfRows / tilesize ) ) };
	std::cout << "Number of tiles horizontal/vertical: " << numberOfHTiles << '/' << numberOfVTiles << '\n';

	// Read whole image into array
	std::cout << "Reading image data ... "; std::cout.flush();
	uint16_t **imageData{ new uint16_t*[asciiFileHeader.numberOfRows] };
	for( uint32_t i{0}; i < asciiFileHeader.numberOfRows; ++i ) {
		imageData[i] = new uint16_t[asciiFileHeader.numberOfColumns];
		for( uint32_t j{0}; j < asciiFileHeader.numberOfColumns; ++j ) {
			std::string s;
			inFile >> s;
			/* Set no data to 0 but this can cause holes in some areas where there is a no data value,
			 * e.g. on some glaciers or where it was particularly cloudy, which happens in the srtm data.
			 * Also clip negative values to 0; it is often sea surface.
			 * Real negative height values below the reference ellipsoid's surface are excluded. */
			int color{ std::stoi( s ) };
			if( color == asciiFileHeader.noDataValue || color < 0 )
				color = 0;
			if( color > 65535 )
				color = 65535;
			imageData[i][j] = static_cast<uint16_t>( color );
		}
	}
	inFile.close();
	std::cout << "done\n";

	std::cout << "Converting images ..." << std::endl;
	/* Calculate start rows/columns for each tile. The last column/top row must overlap so that the new one
	 * starts on the row/column on which the old one ended or there will be gaps between tiles when rendering.
	 * @fixme: Apart from this tiles should overlap by 1 on each side because of normal calculation from
	 * averaging over adjacent posts and sobel filtering. See shaders of terrain lod. */
	const uint32_t numTiles{ numberOfHTiles * numberOfVTiles };
	uint32_t *startRows{ new uint32_t[numTiles] };
	uint32_t *startColumns{ new uint32_t[numTiles] };
	int k{0};
	for( uint32_t i{0}; i < numberOfHTiles; ++i ) {
		for( uint32_t j{0}; j < numberOfVTiles; ++j ) {
			startRows[k] = i * ( tilesize - 1 );
			startColumns[k] = j * ( tilesize - 1 );
			k++;
		}
	}

	/* Go through the data tile by tile.
	 * Calculate minimum and maximum heights for each tile and the cartesian coordinates of each tile's
	 * bounding boxes, originating from the lower left geodetic coordinates and applying the cellsize.
	 * We're strictly assuming square tiles. */
	double minY{ 65535.0 };
	double maxY{ 0.0 };
	for( uint32_t tileNumber{0}; tileNumber < numTiles; ++tileNumber ) {
		png::image<png::gray_pixel_16> image( (uint32_t)tilesize, (uint32_t)tilesize );
		// start reading
		for( uint32_t row{ startRows[tileNumber] }; row < startRows[tileNumber] + tilesize; ++row ) {
			for( uint32_t column{ startColumns[tileNumber] }; column < startColumns[tileNumber] + tilesize; ++column ) {
				uint16_t value{ imageData[column][row] };
				image[ column - startColumns[tileNumber] ]
					 [ row - startRows[tileNumber] ] = png::gray_pixel_16( value ) ;
				if( minY > value )
					minY = value;
				if( maxY < value )
					maxY = value;
			}
		}
		std::ostringstream fileToWrite;
		fileToWrite << "tile_" << tilesize << '_' << (tileNumber+1);
		std::cout << "\tWriting " << fileToWrite.str() << ".png\n";
		image.write( fileToWrite.str() + ".png" );

		std::ofstream outFile{ fileToWrite.str() + ".bb" };
		/* Construct bounding box relative to input data (beginning 0/0/0).
		 * This is used to calculate texture positions during rendering. */
		const double minX{ static_cast<double>( startColumns[tileNumber] ) };
		const double minZ{ static_cast<double>( startRows[tileNumber] ) };
		const double maxX{ static_cast<double>( (tilesize - 1 + startColumns[tileNumber]) ) };
		const double maxZ{ static_cast<double>( (tilesize - 1 + startRows[tileNumber]) ) };
		// Write axis aligned bounding box relative to heightmap data
		outFile << minX << ' ' << minY << ' ' << minZ << ' ' << maxX << ' ' << maxY << ' ' << maxZ << std::endl;

		/* Construct real world bounding box minimum and maximum from geodetic coordinates:
		 * geodetic lower left x + startColumn[tileNumber] * geodetic cellsize,
		 * minimum height,
		 * geodetic lower left y + startRow[tileNumber] * geodetic cellsize,
		 * geodetic lower left x + startColumn[tileNumber] * geodetic cellsize + (tilesize-1) * geodetic cellsize,
		 * maximum height,
		 * geodetic lower left y + startRow[tileNumber] * geodetic cellsize + (tilesize-1) * geodetic cellsize */
		const double minLong{
			asciiFileHeader.llLong + static_cast<double>( startColumns[tileNumber] ) * asciiFileHeader.cellsize
		};
		const double minLat{
			asciiFileHeader.llLat + static_cast<double>( startRows[tileNumber] ) * asciiFileHeader.cellsize
		};
		outFile << std::fixed <<
				minLong << ' ' << minLat << ' ' << asciiFileHeader.cellsize << std::endl;
		outFile.close();
		std::ostringstream bbout;
		bbout << "\tAABB relative to data tile: " <<
				"Min: " << minX << '/' << minY << '/' << minZ << "; " <<
				"Max: " << maxX << '/' << maxY << '/' << maxZ << '\n';
		bbout << std::fixed << "\tGeodetic lower left and cellsize of tile: " <<
				"Lon/Lat: " << minLong << '/' << minLat << "; Cellsize in arcsec: " <<
				asciiFileHeader.cellsize << std::endl;
		std::cout << bbout.str();
	}

	delete [] startColumns;
	delete [] startRows;
	for( uint32_t i{0}; i < asciiFileHeader.numberOfRows; ++i )
		delete [] imageData[i];
	delete [] imageData;

	std::cout << "Program ending" << std::endl;
	return EXIT_SUCCESS;
}

}	// namespace
