
#pragma once

#include <noise/BaseNoise.h>

namespace noise {

/// Noise module that outputs three-dimensional "billowy" noise.
///
/// @image html modulebillow.png
///
/// This noise module generates "billowy" noise suitable for clouds and
/// rocks.
///
/// This noise module is nearly identical to noise::module::Perlin except
/// this noise module modifies each octave with an absolute-value
/// function.  See the documentation of noise::module::Perlin for more
/// information.
class Billow : public BaseNoise {
public:
	Billow( const double frequency = DEFAULT_FREQUENCY, const double lacunarity = DEFAULT_LACUNARITY,
			const double persistence = DEFAULT_PERSISTENCE,	const int octaveCount = DEFAULT_OCTAVE_COUNT,
			const noiseQuality_t quality = DEFAULT_QUALITY,	const int seed = DEFAULT_SEED ) :
				BaseNoise{ frequency, lacunarity, persistence, octaveCount, quality, seed } {}

	virtual double getValue( double x, double y, double z ) const override final {
		double value = 0.0;
		double signal = 0.0;
		double curPersistence = 1.0;
		int seed;
		x *= m_frequency;
		y *= m_frequency;
		z *= m_frequency;
		for( int curOctave{0}; curOctave < m_octaveCount; ++curOctave ) {
			// Make sure that these floating-point values have the same range as a 32-
			// bit integer so that we can pass them to the coherent-noise functions.
			const double nx{ makeInt32Range( x ) };
			const double ny{ makeInt32Range( y ) };
			const double nz{ makeInt32Range( z ) };
			// Get the coherent-noise value from the input value and add it to the final result.
			seed = (m_seed + curOctave) & 0xffffffff;
			signal = gradientCoherentNoise3D (nx, ny, nz, seed, m_quality);
			signal = 2.0 * std::abs( signal ) - 1.0;
			value += signal * curPersistence;
			// Prepare the next octave.
			x *= m_lacunarity;
			y *= m_lacunarity;
			z *= m_lacunarity;
			curPersistence *= m_persistence;
		}
		return value += 0.5;
	}

};

}
