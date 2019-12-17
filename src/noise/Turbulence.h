
#pragma once

#include <noise/BaseNoise.h>

namespace noise {

/// Noise module that randomly displaces the input value before
/// returning the output value from a source module.
///
/// @image html moduleturbulence.png
///
/// @a Turbulence is the pseudo-random displacement of the input value.
/// The GetValue() method randomly displaces the ( @a x, @a y, @a z )
/// coordinates of the input value before retrieving the output value from
/// the source module.  To control the turbulence, an application can
/// modify its frequency, its power, and its roughness.
///
/// The frequency of the turbulence determines how rapidly the
/// displacement amount changes.  To specify the frequency, call the
/// SetFrequency() method.
///
/// The power of the turbulence determines the scaling factor that is
/// applied to the displacement amount.  To specify the power, call the
/// SetPower() method.
///
/// The roughness of the turbulence determines the roughness of the
/// changes to the displacement amount.  Low values smoothly change the
/// displacement amount.  High values roughly change the displacement
/// amount, which produces more "kinky" changes.  To specify the
/// roughness, call the SetRoughness() method.
///
/// Use of this noise module may require some trial and error.  Assuming
/// that you are using a generator module as the source module, you
/// should first:
/// - Set the frequency to the same frequency as the source module.
/// - Set the power to the reciprocal of the frequency.
///
/// From these initial frequency and power values, modify these values
/// until this noise module produce the desired changes in your terrain or
/// texture.  For example:
/// - Low frequency (1/8 initial frequency) and low power (1/8 initial
///   power) produces very minor, almost unnoticeable changes.
/// - Low frequency (1/8 initial frequency) and high power (8 times
///   initial power) produces "ropey" lava-like terrain or marble-like
///   textures.
/// - High frequency (8 times initial frequency) and low power (1/8
///   initial power) produces a noisy version of the initial terrain or
///   texture.
/// - High frequency (8 times initial frequency) and high power (8 times
///   initial power) produces nearly pure noise, which isn't entirely
///   useful.
///
/// Displacing the input values result in more realistic terrain and
/// textures.  If you are generating elevations for terrain height maps,
/// you can use this noise module to produce more realistic mountain
/// ranges or terrain features that look like flowing lava rock.  If you
/// are generating values for textures, you can use this noise module to
/// produce realistic marble-like or "oily" textures.
///
/// Internally, there are three noise::module::Perlin noise modules
/// that displace the input value; one for the @a x, one for the @a y,
/// and one for the @a z coordinate.
///
/// The roughness of the turbulence determines the roughness of the
/// changes to the displacement amount.  Low values smoothly change
/// the displacement amount.  High values roughly change the
/// displacement amount, which produces more "kinky" changes.
///
/// Internally, there are three noise::module::Perlin noise modules
/// that displace the input value; one for the @a x, one for the @a y,
/// and one for the @a z coordinate.  The roughness value is equal to
/// the number of octaves used by the noise::module::Perlin noise
/// modules.
///

///
/// This noise module requires one source module.
class Turbulence : public BaseNoise {
public:
	/// Default power for the noise::module::Turbulence noise module.
	static constexpr double DEFAULT_POWER{ 1.0 };

	/// Default roughness for the noise::module::Turbulence noise module.
	static constexpr int DEFAULT_ROUGHNESS{ 3 };

	Turbulence( BaseNoise *source = nullptr,
				const double frequency = DEFAULT_FREQUENCY,
				const double power = DEFAULT_POWER,
				const int roughness = DEFAULT_ROUGHNESS,
				const int seed = DEFAULT_SEED ) :
					BaseNoise{}, m_power{power}, m_source{source} {
		if( m_source == nullptr )
			throw std::runtime_error( "Parameter error in turbulence module." );
		setSeed( seed );
		setFrequency( frequency );
		setOctaveCount( roughness );
		// Set the seed of each noise::module::Perlin noise modules.  To prevent any
		// sort of weird artifacting, use a slightly different seed for each noise
		// module.
		m_xDistortion.setSeed( m_seed );
		m_yDistortion.setSeed( m_seed + 1 );
		m_zDistortion.setSeed( m_seed + 2 );
		m_xDistortion.setFrequency( frequency );
		m_yDistortion.setFrequency( frequency );
		m_zDistortion.setFrequency( frequency );
		m_xDistortion.setOctaveCount( roughness );
		m_yDistortion.setOctaveCount( roughness );
		m_zDistortion.setOctaveCount( roughness );
	}

	virtual double getValue( double x, double y, double z ) const override final {
		// Get the values from the three noise::module::Perlin noise modules and
		// add each value to each coordinate of the input value.  There are also
		// some offsets added to the coordinates of the input values.  This prevents
		// the distortion modules from returning zero if the (x, y, z) coordinates,
		// when multiplied by the frequency, are near an integer boundary.  This is
		// due to a property of gradient coherent noise, which returns zero at
		// integer boundaries.
		const double x0{ x + ( 12414.0 / 65536.0 ) };
		const double y0{ y + ( 65124.0 / 65536.0 ) };
		const double z0{ z + ( 31337.0 / 65536.0 ) };
		const double x1{ x + ( 26519.0 / 65536.0 ) };
		const double y1{ y + ( 18128.0 / 65536.0 ) };
		const double z1{ z + ( 60493.0 / 65536.0 ) };
		const double x2{ x + ( 53820.0 / 65536.0 ) };
		const double y2{ y + ( 11213.0 / 65536.0 ) };
		const double z2{ z + ( 44845.0 / 65536.0 ) };
		const double xDistort{ x + ( m_xDistortion.getValue( x0, y0, z0 ) * m_power ) };
		const double yDistort{ y + ( m_yDistortion.getValue( x1, y1, z1 ) * m_power ) };
		const double zDistort{ z + ( m_zDistortion.getValue( x2, y2, z2 ) * m_power ) };
		// Retrieve the output value at the offsetted input value instead of the
		// original input value.
		return m_source->getValue( xDistort, yDistort, zDistort );
	}

private:
	/// The power (scale) of the displacement.
	double m_power = DEFAULT_POWER;

	BaseNoise *m_source{ nullptr };

	/// Noise module that displaces the x coordinate.
	Perlin m_xDistortion;

	/// Noise module that displaces the y coordinate.
	Perlin m_yDistortion;

	/// Noise module that displaces the z coordinate.
	Perlin m_zDistortion;

};

}
