
#pragma once

#include <noise/BaseNoise.h>

namespace noise {

/// Noise module that outputs 3-dimensional ridged-multifractal noise.
///
/// @image html moduleridgedmulti.png
///
/// This noise module, heavily based on the Perlin-noise module, generates
/// ridged-multifractal noise.  Ridged-multifractal noise is generated in
/// much of the same way as Perlin noise, except the output of each octave
/// is modified by an absolute-value function.  Modifying the octave
/// values in this way produces ridge-like formations.
///
/// Ridged-multifractal noise does not use a persistence value.  This is
/// because the persistence values of the octaves are based on the values
/// generated from from previous octaves, creating a feedback loop (or
/// that's what it looks like after reading the code.)
///
/// This noise module outputs ridged-multifractal-noise values that
/// usually range from -1.0 to +1.0, but there are no guarantees that all
/// output values will exist within that range.
///
/// @note For ridged-multifractal noise generated with only one octave,
/// the output value ranges from -1.0 to 0.0.
///
/// Ridged-multifractal noise is often used to generate craggy mountainous
/// terrain or marble-like textures.
///
/// This noise module does not require any source modules.
///
/// <b>Octaves</b>
///
/// The number of octaves control the <i>amount of detail</i> of the
/// ridged-multifractal noise.  Adding more octaves increases the detail
/// of the ridged-multifractal noise, but with the drawback of increasing
/// the calculation time.
///
/// An application may specify the number of octaves that generate
/// ridged-multifractal noise by calling the SetOctaveCount() method.
///
/// <b>Frequency</b>
///
/// An application may specify the frequency of the first octave by
/// calling the SetFrequency() method.
///
/// <b>Lacunarity</b>
///
/// The lacunarity specifies the frequency multipler between successive
/// octaves.
///
/// The effect of modifying the lacunarity is subtle; you may need to play
/// with the lacunarity value to determine the effects.  For best results,
/// set the lacunarity to a number between 1.5 and 3.5.
///
/// <b>References &amp; Acknowledgments</b>
///
/// <a href=http://www.texturingandmodeling.com/Musgrave.html>F.
/// Kenton "Doc Mojo" Musgrave's texturing page</a> - This page contains
/// links to source code that generates ridged-multfractal noise, among
/// other types of noise.  The source file <a
/// href=http://www.texturingandmodeling.com/CODE/MUSGRAVE/CLOUD/fractal.c>
/// fractal.c</a> contains the code I used in my ridged-multifractal class
/// (see the @a RidgedMultifractal() function.)  This code was written by F.
/// Kenton Musgrave, the person who created
/// <a href=http://www.pandromeda.com/>MojoWorld</a>.  He is also one of
/// the authors in <i>Texturing and Modeling: A Procedural Approach</i>
/// (Morgan Kaufmann, 2002. ISBN 1-55860-848-6.)
class RidgeMulti : public BaseNoise {
public:

	RidgeMulti( const double frequency = DEFAULT_FREQUENCY, const double lacunarity = DEFAULT_LACUNARITY,
				const int octaveCount = DEFAULT_OCTAVE_COUNT, const noiseQuality_t quality = DEFAULT_QUALITY,
				const int seed = DEFAULT_SEED ) :
					BaseNoise{ frequency, lacunarity, DEFAULT_PERSISTENCE, octaveCount, quality, seed } {
		calcSpectralWeights();
	}

	virtual ~RidgeMulti() {}

	// Multifractal code originally written by F. Kenton "Doc Mojo" Musgrave,
	// 1998.  Modified by jas for use with libnoise.
	virtual double getValue( double x, double y, double z ) const override final {
		x *= m_frequency;
		y *= m_frequency;
		z *= m_frequency;
		double value{ 0.0 };
		double weight{ 1.0 };
		// These parameters should be user-defined; may be exposed in the future.
		double offset = 1.0;
		double gain = 2.0;
		for( int curOctave{0}; curOctave < m_octaveCount; ++curOctave ) {
			// Make sure that these floating-point values have the same range as a 32-
			// bit integer so that we can pass them to the coherent-noise functions.
			const double nx{ makeInt32Range(x) };
			const double ny{ makeInt32Range(y) };
			const double nz{ makeInt32Range(z) };
			// Get the coherent-noise value.
			int seed = (m_seed + curOctave) & 0x7fffffff;
			double signal{ gradientCoherentNoise3D( nx, ny, nz, seed, m_quality ) };
			// Make the ridges.
			signal = std::abs( signal );
			signal = offset - signal;
			// Square the signal to increase the sharpness of the ridges.
			signal *= signal;
			// The weighting from the previous octave is applied to the signal.
			// Larger values have higher weights, producing sharp points along the
			// ridges.
			signal *= weight;
			// Weight successive contributions by the previous signal.
			weight = signal * gain;
			if( weight > 1.0 )
				weight = 1.0;
			if( weight < 0.0 )
				weight = 0.0;
			// Add the signal to the output value.
			value += ( signal * m_spectralWeights[curOctave] );
			// Go to the next octave.
			x *= m_lacunarity;
			y *= m_lacunarity;
			z *= m_lacunarity;
		}
		return value * 1.25 - 1.0;
	}

private:
	/// Calculates the spectral weights for each octave.
	/// This method is called when the lacunarity changes.
	void calcSpectralWeights() {
		// The exponent should be user-defined; may be exposed later.
		double h{ 1.0 };
		double frequency{ 1.0 };
		for( int i{0}; i < MAX_OCTAVE_COUNT; ++i ) {
			// Compute weight for each frequency.
			m_spectralWeights[i] = std::pow( frequency, -h );
			frequency *= m_lacunarity;
		}
	}

	/// Contains the spectral weights for each octave.
	double m_spectralWeights[MAX_OCTAVE_COUNT];

};

}
