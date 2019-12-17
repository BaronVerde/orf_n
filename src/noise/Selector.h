
#pragma once

#include <noise/BaseNoise.h>

namespace noise {

/// Noise module that outputs the value selected from one of two source
/// modules chosen by the output value from a control module.
///
/// @image html moduleselect.png
///
/// Unlike most other noise modules, the index value assigned to a source
/// module determines its role in the selection operation:
/// - Source module 0 (upper left in the diagram) outputs a value.
/// - Source module 1 (lower left in the diagram) outputs a value.
/// - Source module 2 (bottom of the diagram) is known as the <i>control
///   module</i>.  The control module determines the value to select.  If
///   the output value from the control module is within a range of values
///   known as the <i>selection range</i>, this noise module outputs the
///   value from the source module with an index value of 1.  Otherwise,
///   this noise module outputs the value from the source module with an
///   index value of 0.
///
/// To specify the bounds of the selection range, call the SetBounds()
/// method.
///
/// An application can pass the control module to the SetControlModule()
/// method instead of the SetSourceModule() method.  This may make the
/// application code easier to read.
///
/// By default, there is an abrupt transition between the output values
/// from the two source modules at the selection-range boundary.  To
/// smooth the transition, pass a non-zero value to the SetEdgeFalloff()
/// method.  Higher values result in a smoother transition.
/// For example, if the selection range is 0.5 to 0.8, and the edge
/// falloff value is 0.1, then the GetValue() method outputs:
/// - the output value from the source module with an index value of 0
///   if the output value from the control module is less than 0.4
///   ( = 0.5 - 0.1).
/// - a linear blend between the two output values from the two source
///   modules if the output value from the control module is between
///   0.4 ( = 0.5 - 0.1) and 0.6 ( = 0.5 + 0.1).
/// - the output value from the source module with an index value of 1
///   if the output value from the control module is between 0.6
///   ( = 0.5 + 0.1) and 0.7 ( = 0.8 - 0.1).
/// - a linear blend between the output values from the two source
///   modules if the output value from the control module is between
///   0.7 ( = 0.8 - 0.1 ) and 0.9 ( = 0.8 + 0.1).
/// - the output value from the source module with an index value of 0
///   if the output value from the control module is greater than 0.9
///   ( = 0.8 + 0.1).
///
/// If the output value from the control module is within the
/// selection range, the GetValue() method outputs the value from the
/// source module with an index value of 1.  Otherwise, this method
/// outputs the value from the source module with an index value of 0.
///
/// This noise module requires three source modules.
class Selector : public BaseNoise {
	/// Default edge-falloff value for the noise::module::Select noise module.
	static constexpr double DEFAULT_EDGE_FALLOFF{ 0.0 };

	/// Default lower bound of the selection range for the
	/// noise::module::Select noise module.
	static constexpr double DEFAULT_LOWER_BOUND{ -1.0 };

	/// Default upper bound of the selection range for the
	/// noise::module::Select noise module.
	static constexpr double DEFAULT_UPPER_BOUND{ 1.0 };

public:
	Selector( const BaseNoise *source1,
			  const BaseNoise *source2,
			  const BaseNoise *control,
			  const double lowerBound = DEFAULT_LOWER_BOUND,
			  const double upperBound = DEFAULT_UPPER_BOUND,
			  const double edgeFalloff = DEFAULT_EDGE_FALLOFF ) :
				  BaseNoise{},
				  m_source1{source1}, m_source2{source2}, m_control{control},
				  m_lowerBound{lowerBound}, m_upperBound{upperBound}, m_edgeFalloff{edgeFalloff} {
		if( m_source1 == nullptr || m_source2 == nullptr ||
			m_control == nullptr ||	m_lowerBound >= m_upperBound )
			throw std::runtime_error( "Parameter error in selector module." );
		// Make sure that the edge falloff curves do not overlap.
		double boundSize{ m_upperBound - m_lowerBound };
		m_edgeFalloff = ( edgeFalloff > boundSize / 2 ) ? boundSize / 2: edgeFalloff;
	}

	virtual ~Selector() {}

	virtual double getValue( double x, double y, double z ) const override final {
		const double controlValue{ m_control->getValue( x, y, z ) };
		if( m_edgeFalloff > 0.0 ) {
			if( controlValue < ( m_lowerBound - m_edgeFalloff ) )
				// The output value from the control module is below the selector
				// threshold; return the output value from the first source module.
				return m_source1->getValue( x, y, z );
			else if( controlValue < ( m_lowerBound + m_edgeFalloff ) ) {
				// The output value from the control module is near the lower end of the
				// selector threshold and within the smooth curve. Interpolate between
				// the output values from the first and second source modules.
				const double lowerCurve{ m_lowerBound - m_edgeFalloff };
				const double upperCurve{ m_lowerBound + m_edgeFalloff };
				const double alpha = sCurve3( ( controlValue - lowerCurve ) / ( upperCurve - lowerCurve ) );
				return noiselerp( m_source1->getValue( x, y, z ), m_source2->getValue( x, y, z ), alpha );
			} else if( controlValue < ( m_upperBound - m_edgeFalloff ) )
				// The output value from the control module is within the selector
				// threshold; return the output value from the second source module.
				return m_source2->getValue( x, y, z );
			else if( controlValue < ( m_upperBound + m_edgeFalloff ) ) {
				// The output value from the control module is near the upper end of the
				// selector threshold and within the smooth curve. Interpolate between
				// the output values from the first and second source modules.
				const double lowerCurve{ m_upperBound - m_edgeFalloff };
				const double upperCurve{ m_upperBound + m_edgeFalloff };
				const double alpha{ sCurve3( ( controlValue - lowerCurve ) / ( upperCurve - lowerCurve ) ) };
				return noiselerp( m_source2->getValue( x, y, z ), m_source1->getValue( x, y, z ), alpha );
			} else
				// Output value from the control module is above the selector threshold;
				// return the output value from the first source module.
				return m_source1->getValue( x, y, z );
		} else {
			if( controlValue < m_lowerBound || controlValue > m_upperBound )
				return m_source1->getValue( x, y, z );
			else
				return m_source2->getValue( x, y, z );
		}
	}

private:
	const BaseNoise *m_source1{ nullptr };

	const BaseNoise *m_source2{ nullptr };

	const BaseNoise *m_control{ nullptr };

	double m_lowerBound{ DEFAULT_LOWER_BOUND };

	double m_upperBound{ DEFAULT_UPPER_BOUND };

	double m_edgeFalloff{ DEFAULT_EDGE_FALLOFF };

};

}
