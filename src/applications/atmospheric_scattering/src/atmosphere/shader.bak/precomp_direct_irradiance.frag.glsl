
#version 450 core

layout( location = 0 ) out vec3 delta_irradiance;
layout( location = 1 ) out vec3 irradiance;

uniform sampler2D transmittance_texture;

const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;
const int IRRADIANCE_TEXTURE_WIDTH = 64;
const int IRRADIANCE_TEXTURE_HEIGHT = 16;

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

float GetTextureCoordFromUnitRange( float x, int texture_size ) {
	return 0.5f / float( texture_size ) + x * ( 1.0f - 1.0f / float(texture_size) );
}

float GetUnitRangeFromTextureCoord( float u, int texture_size ) {
	return ( u - 0.5f / float(texture_size) ) / ( 1.0f - 1.0f / float(texture_size) );
}

float DistanceToTopAtmosphereBoundary( float r, float mu ) {
	float discriminant = r * r * (mu * mu - 1.0f) + atmosphere.top_radius * atmosphere.top_radius;
	return ClampDistance( -r * mu + SafeSqrt(discriminant ) );
}

vec2 GetTransmittanceTextureUvFromRMu( float r, float mu ) {
	float H = sqrt( atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float rho = SafeSqrt( r * r - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float d = DistanceToTopAtmosphereBoundary( r, mu );
	float d_min = atmosphere.top_radius - r;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;
	return vec2( GetTextureCoordFromUnitRange( x_mu, TRANSMITTANCE_TEXTURE_WIDTH ),
				 GetTextureCoordFromUnitRange( x_r, TRANSMITTANCE_TEXTURE_HEIGHT ) );
}

vec3 GetTransmittanceToTopAtmosphereBoundary( float r, float mu ) {
	vec2 uv = GetTransmittanceTextureUvFromRMu( r, mu );
	return vec3( texture( transmittance_texture, uv ) );
}

vec3 ComputeDirectIrradiance( float r, float mu_s) {
	float alpha_s = atmosphere.sun_angular_radius / rad;
	float average_cosine_factor = mu_s < -alpha_s ? 
		0.0f : (mu_s > alpha_s ? mu_s : (mu_s + alpha_s) * (mu_s + alpha_s) / (4.0f * alpha_s));
	return atmosphere.solar_irradiance * GetTransmittanceToTopAtmosphereBoundary( r, mu_s ) * average_cosine_factor;
}

void GetRMuSFromIrradianceTextureUv( in vec2 uv, out float r, out float mu_s ) {
	float x_mu_s = GetUnitRangeFromTextureCoord( uv.x, IRRADIANCE_TEXTURE_WIDTH );
	float x_r = GetUnitRangeFromTextureCoord( uv.y, IRRADIANCE_TEXTURE_HEIGHT );
	r = atmosphere.bottom_radius + x_r * (atmosphere.top_radius - atmosphere.bottom_radius);
	mu_s = ClampCosine( 2.0f * x_mu_s - 1.0f );
}

const vec2 IRRADIANCE_TEXTURE_SIZE = vec2( IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT );
vec3 ComputeDirectIrradianceTexture( in vec2 frag_coord ) {
	float r;
	float mu_s;
	GetRMuSFromIrradianceTextureUv( frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s );
	return ComputeDirectIrradiance( r, mu_s );
}

void main() {
	delta_irradiance = ComputeDirectIrradianceTexture( gl_FragCoord.xy );
	irradiance = vec3( 0.0f );
}

