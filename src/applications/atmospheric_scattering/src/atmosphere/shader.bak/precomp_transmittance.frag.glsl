
#version 450 core

layout(location = 0) out vec3 transmittance;

const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

const float m = 1.0;
const float nm = 1.0;
const float rad = 1.0;
const float sr = 1.0;
const float watt = 1.0;
const float lm = 1.0;
const float PI = 3.14159265358979323846;
const float km = 1000.0 * m;
const float m2 = m * m;
const float m3 = m * m * m;
const float pi = PI * rad;
const float deg = pi / 180.0;
const float watt_per_square_meter = watt / m2;
const float watt_per_square_meter_per_sr = watt / (m2 * sr);
const float watt_per_square_meter_per_nm = watt / (m2 * nm);
const float watt_per_square_meter_per_sr_per_nm = watt / (m2 * sr * nm);
const float watt_per_cubic_meter_per_sr_per_nm = watt / (m3 * sr * nm);
const float cd = lm / sr;
const float kcd = 1000.0 * cd;
const float cd_per_square_meter = cd / m2;
const float kcd_per_square_meter = kcd / m2;

struct DensityProfileLayer {
	float width;
	float exp_term;
	float exp_scale;
	float linear_term;
	float constant_term;
};

struct DensityProfile {
	DensityProfileLayer layers[2];
};

struct AtmosphereParameters {
	vec3 solar_irradiance;
	float sun_angular_radius;
	float bottom_radius;
	float top_radius;
	DensityProfile rayleigh_density;
	vec3 rayleigh_scattering;
	DensityProfile mie_density;
	vec3 mie_scattering;
	vec3 mie_extinction;
	float mie_phase_function_g;
	DensityProfile absorption_density;
	vec3 absorption_extinction;
	vec3 ground_albedo;
	float mu_s_min;
};

const AtmosphereParameters atmosphere = AtmosphereParameters(
	vec3(1.474000,1.850400,1.911980),
	0.004675, 6360.000000, 6420.000000,
	DensityProfile( DensityProfileLayer[2]
		( DensityProfileLayer(0.000000,0.000000,0.000000,0.000000,0.000000),
		DensityProfileLayer(0.000000,1.000000,-0.125000,0.000000,0.000000) )
	),
	vec3(0.005802,0.013558,0.033100),
	DensityProfile( DensityProfileLayer[2]
		( DensityProfileLayer(0.000000,0.000000,0.000000,0.000000,0.000000),
		DensityProfileLayer(0.000000,1.000000,-0.833333,0.000000,0.000000) )
	),
	vec3(0.003996,0.003996,0.003996),
	vec3(0.004440,0.004440,0.004440),
	0.800000,
	DensityProfile(DensityProfileLayer[2]
		( DensityProfileLayer(25.000000,0.000000,0.000000,0.066667,-0.666667),
		DensityProfileLayer(0.000000,0.000000,0.000000,-0.066667,2.666667) )
	),
	vec3(0.000650,0.001881,0.000085),
	vec3(0.100000,0.100000,0.100000),
	-0.500000
);

float ClampCosine( float mu ) {
	return clamp( mu, -1.0f, 1.0f );
}
float ClampDistance( float d ) {
	return max( d, 0.0f * m );
}

float SafeSqrt( float a ) {
	return sqrt( max( a, 0.0f * m2 ) );
}

float GetLayerDensity( in DensityProfileLayer layer, float altitude ) {
	float density = layer.exp_term * exp( layer.exp_scale * altitude ) + layer.linear_term * altitude + layer.constant_term;
	return clamp( density, 0.0f, 1.0f );
}

float GetProfileDensity( in DensityProfile profile, float altitude ) {
  return altitude < profile.layers[0].width ?
      GetLayerDensity( profile.layers[0], altitude ) : GetLayerDensity( profile.layers[1], altitude );
}

float DistanceToTopAtmosphereBoundary( float r, float mu ) {
	float discriminant = r * r * (mu * mu - 1.0f) + atmosphere.top_radius * atmosphere.top_radius;
	return ClampDistance( -r * mu + SafeSqrt( discriminant ) );
}

float ComputeOpticalLengthToTopAtmosphereBoundary( in DensityProfile profile, float r, float mu ) {
	const int SAMPLE_COUNT = 500;
	float dx = DistanceToTopAtmosphereBoundary( r, mu) / float(SAMPLE_COUNT );
	float result = 0.0f * m;
	for( int i = 0; i <= SAMPLE_COUNT; ++i ) {
		float d_i = float(i) * dx;
		float r_i = sqrt(d_i * d_i + 2.0f * r * mu * d_i + r * r);
		float y_i = GetProfileDensity( profile, r_i - atmosphere.bottom_radius );
		float weight_i = i == 0 || i == SAMPLE_COUNT ? 0.5f : 1.0f;
		result += y_i * weight_i * dx;
	}
	return result;
}

float GetTextureCoordFromUnitRange( float x, int texture_size ) {
	return 0.5f / float( texture_size ) + x * ( 1.0f - 1.0f / float(texture_size) );
}

float GetUnitRangeFromTextureCoord( float u, int texture_size ) {
  return ( u - 0.5f / float(texture_size) ) / ( 1.0f - 1.0f / float(texture_size) );
}

void GetRMuFromTransmittanceTextureUv( in vec2 uv, out float r, out float mu ) {
	float x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);
	float x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);
	float H = sqrt( atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float rho = H * x_r;
	r = sqrt( rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius );
	float d_min = atmosphere.top_radius - r;
	float d_max = rho + H;
	float d = d_min + x_mu * (d_max - d_min);
	mu = d == 0.0f * m ? 1.0f : (H * H - rho * rho - d * d) / (2.0f * r * d);
	mu = ClampCosine(mu);
}

vec3 ComputeTransmittanceToTopAtmosphereBoundary( float r, float mu ) {
	return exp( -( atmosphere.rayleigh_scattering *
		ComputeOpticalLengthToTopAtmosphereBoundary( atmosphere.rayleigh_density, r, mu ) +
		atmosphere.mie_extinction *
		ComputeOpticalLengthToTopAtmosphereBoundary( atmosphere.mie_density, r, mu ) +
		atmosphere.absorption_extinction *
		ComputeOpticalLengthToTopAtmosphereBoundary( atmosphere.absorption_density, r, mu ) ) );
}

vec3 ComputeTransmittanceToTopAtmosphereBoundaryTexture( in vec2 frag_coord ) {
	const vec2 TRANSMITTANCE_TEXTURE_SIZE = vec2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
	float r;
	float mu;
	GetRMuFromTransmittanceTextureUv( frag_coord / TRANSMITTANCE_TEXTURE_SIZE, r, mu );
	return ComputeTransmittanceToTopAtmosphereBoundary( r, mu );
}

void main() {
	transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture( gl_FragCoord.xy );
}

