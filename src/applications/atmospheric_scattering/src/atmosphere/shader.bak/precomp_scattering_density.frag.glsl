
#version 450 core

layout( location = 0 ) out vec3 delta_rayleigh;
layout( location = 1 ) out vec3 delta_mie;
layout( location = 2 ) out vec4 scattering;
layout( location = 3 ) out vec3 single_mie_scattering;
uniform mat3 luminance_from_radiance;
uniform sampler2D transmittance_texture;
uniform int layer;

const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;
const int SCATTERING_TEXTURE_R_SIZE = 32;
const int SCATTERING_TEXTURE_MU_SIZE = 128;
const int SCATTERING_TEXTURE_MU_S_SIZE = 32;
const int SCATTERING_TEXTURE_NU_SIZE = 8;

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

float ClampRadius( float r ) {
	return clamp( r, atmosphere.bottom_radius, atmosphere.top_radius );
}

float GetTextureCoordFromUnitRange( float x, int texture_size ) {
	return 0.5f / float( texture_size ) + x * ( 1.0f - 1.0f / float(texture_size) );
}

float GetLayerDensity( in DensityProfileLayer layer, float altitude ) {
	float density = layer.exp_term * exp( layer.exp_scale * altitude ) + layer.linear_term * altitude + layer.constant_term;
	return clamp( density, 0.0f, 1.0f );
}

float GetProfileDensity( in DensityProfile profile, float altitude ) {
  return altitude < profile.layers[0].width ?
      GetLayerDensity( profile.layers[0], altitude ) : GetLayerDensity( profile.layers[1], altitude );
}

float GetUnitRangeFromTextureCoord( float u, int texture_size ) {
	return ( u - 0.5f / float(texture_size) ) / ( 1.0f - 1.0f / float(texture_size) );
}

float DistanceToTopAtmosphereBoundary( float r, float mu ) {
	float discriminant = r * r * (mu * mu - 1.0f) + atmosphere.top_radius * atmosphere.top_radius;
	return ClampDistance( -r * mu + SafeSqrt(discriminant) );
}

float DistanceToBottomAtmosphereBoundary( float r, float mu ) {
	float discriminant = r * r * (mu * mu - 1.0f) + atmosphere.bottom_radius * atmosphere.bottom_radius;
	return ClampDistance( -r * mu - SafeSqrt(discriminant) );
}

float DistanceToNearestAtmosphereBoundary( float r, float mu, bool ray_r_mu_intersects_ground ) {
	if( ray_r_mu_intersects_ground )
		return DistanceToBottomAtmosphereBoundary( r, mu );
	else 
		return DistanceToTopAtmosphereBoundary( r, mu );
}

vec2 GetTransmittanceTextureUvFromRMu( float r, float mu) {
	float H = sqrt( atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float rho = SafeSqrt( r * r - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float d = DistanceToTopAtmosphereBoundary( r, mu );
	float d_min = atmosphere.top_radius - r;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;
	return vec2( GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
				 GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT) );
}

vec3 GetTransmittanceToTopAtmosphereBoundary( float r, float mu) {
	vec2 uv = GetTransmittanceTextureUvFromRMu( r, mu );
	return vec3( texture(transmittance_texture, uv) );
}

vec3 GetTransmittance( float r, float mu, float d, bool ray_r_mu_intersects_ground ) {
	float r_d = ClampRadius( sqrt(d * d + 2.0 * r * mu * d + r * r) );
	float mu_d = ClampCosine((r * mu + d) / r_d);
	if( ray_r_mu_intersects_ground )
		return min( GetTransmittanceToTopAtmosphereBoundary( r_d, -mu_d ) /
					GetTransmittanceToTopAtmosphereBoundary( r, -mu ),
					vec3(1.0f) );
	else
		return min( GetTransmittanceToTopAtmosphereBoundary( r, mu ) /
					GetTransmittanceToTopAtmosphereBoundary( r_d, mu_d ),
					vec3(1.0f) );
}

vec3 GetTransmittanceToSun( float r, float mu_s) {
	float sin_theta_h = atmosphere.bottom_radius / r;
	float cos_theta_h = -sqrt( max(1.0f - sin_theta_h * sin_theta_h, 0.0f) );
	return GetTransmittanceToTopAtmosphereBoundary( r, mu_s ) *
			smoothstep(-sin_theta_h * atmosphere.sun_angular_radius / rad,
						sin_theta_h * atmosphere.sun_angular_radius / rad,
						mu_s - cos_theta_h );
}

void ComputeSingleScatteringIntegrand( float r, float mu, float mu_s, float nu, float d,
	bool ray_r_mu_intersects_ground, out vec3 rayleigh, out vec3 mie) {
	float r_d = ClampRadius( sqrt(d * d + 2.0f * r * mu * d + r * r) );
	float mu_s_d = ClampCosine( (r * mu_s + d * nu) / r_d );
	vec3 transmittance = GetTransmittance( r, mu, d, ray_r_mu_intersects_ground ) *
		GetTransmittanceToSun( r_d, mu_s_d );
	rayleigh = transmittance * GetProfileDensity( atmosphere.rayleigh_density, r_d - atmosphere.bottom_radius );
	mie = transmittance * GetProfileDensity( atmosphere.mie_density, r_d - atmosphere.bottom_radius );
}

void ComputeSingleScattering( float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground,
	out vec3 rayleigh, out vec3 mie) {
	const int SAMPLE_COUNT = 50;
	float dx = DistanceToNearestAtmosphereBoundary( r, mu, ray_r_mu_intersects_ground) / float(SAMPLE_COUNT);
	vec3 rayleigh_sum = vec3(0.0f);
	vec3 mie_sum = vec3(0.0f);
	for( int i = 0; i <= SAMPLE_COUNT; ++i ) {
		float d_i = float(i) * dx;
		vec3 rayleigh_i;
		vec3 mie_i;
		ComputeSingleScatteringIntegrand( r, mu, mu_s, nu, d_i, ray_r_mu_intersects_ground, rayleigh_i, mie_i );
		float weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5f : 1.0f;
		rayleigh_sum += rayleigh_i * weight_i;
		mie_sum += mie_i * weight_i;
	}
	rayleigh = rayleigh_sum * dx * atmosphere.solar_irradiance * atmosphere.rayleigh_scattering;
	mie = mie_sum * dx * atmosphere.solar_irradiance * atmosphere.mie_scattering;
}

void GetRMuMuSNuFromScatteringTextureUvwz( in vec4 uvwz, out float r, out float mu, out float mu_s,
	out float nu, out bool ray_r_mu_intersects_ground ) {
	float H = sqrt( atmosphere.top_radius * atmosphere.top_radius -
					atmosphere.bottom_radius * atmosphere.bottom_radius );
	float rho = H * GetUnitRangeFromTextureCoord(uvwz.w, SCATTERING_TEXTURE_R_SIZE);
	r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
	if (uvwz.z < 0.5) {
		float d_min = r - atmosphere.bottom_radius;
		float d_max = rho;
		float d = d_min + (d_max - d_min) *
			GetUnitRangeFromTextureCoord( 1.0f - 2.0f * uvwz.z, SCATTERING_TEXTURE_MU_SIZE / 2 );
		mu = d == 0.0 * m ? -1.0f : ClampCosine(-(rho * rho + d * d) / (2.0f * r * d));
		ray_r_mu_intersects_ground = true;
	} else {
		float d_min = atmosphere.top_radius - r;
		float d_max = rho + H;
		float d = d_min + (d_max - d_min) *
			GetUnitRangeFromTextureCoord( 2.0f * uvwz.z - 1.0f, SCATTERING_TEXTURE_MU_SIZE / 2 );
		mu = d == 0.0f * m ? 1.0f : ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d) );
		ray_r_mu_intersects_ground = false;
	}
	float x_mu_s = GetUnitRangeFromTextureCoord(uvwz.y, SCATTERING_TEXTURE_MU_S_SIZE);
	float d_min = atmosphere.top_radius - atmosphere.bottom_radius;
	float d_max = H;
	float A = -2.0f * atmosphere.mu_s_min * atmosphere.bottom_radius / (d_max - d_min);
	float a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
	float d = d_min + min(a, A) * (d_max - d_min);
	mu_s = d == 0.0 * m ? 1.0f : ClampCosine((H * H - d * d) / (2.0f * atmosphere.bottom_radius * d));
	nu = ClampCosine(uvwz.x * 2.0f - 1.0f);
}

void GetRMuMuSNuFromScatteringTextureFragCoord( in vec3 frag_coord, out float r, out float mu, 
	out float mu_s, out float nu, out bool ray_r_mu_intersects_ground ) {
	const vec4 SCATTERING_TEXTURE_SIZE = vec4( 
		SCATTERING_TEXTURE_NU_SIZE - 1, SCATTERING_TEXTURE_MU_S_SIZE, SCATTERING_TEXTURE_MU_SIZE, SCATTERING_TEXTURE_R_SIZE
	);
	float frag_coord_nu = floor( frag_coord.x / float(SCATTERING_TEXTURE_MU_S_SIZE) );
	float frag_coord_mu_s = mod( frag_coord.x, float(SCATTERING_TEXTURE_MU_S_SIZE) );
	vec4 uvwz = vec4( frag_coord_nu, frag_coord_mu_s, frag_coord.y, frag_coord.z ) / SCATTERING_TEXTURE_SIZE;
	GetRMuMuSNuFromScatteringTextureUvwz( uvwz, r, mu, mu_s, nu, ray_r_mu_intersects_ground );
	nu = clamp( nu, mu * mu_s - 
		sqrt((1.0f - mu * mu) * (1.0f - mu_s * mu_s)), mu * mu_s + sqrt((1.0f - mu * mu) * (1.0f - mu_s * mu_s))
	);
}

void ComputeSingleScatteringTexture( in vec3 frag_coord, out vec3 rayleigh, out vec3 mie ) {
	float r;
	float mu;
	float mu_s;
	float nu;
	bool ray_r_mu_intersects_ground;
	GetRMuMuSNuFromScatteringTextureFragCoord( frag_coord, r, mu, mu_s, nu, ray_r_mu_intersects_ground );
	ComputeSingleScattering( r, mu, mu_s, nu, ray_r_mu_intersects_ground, rayleigh, mie );
}

void main() {
	ComputeSingleScatteringTexture( vec3(gl_FragCoord.xy, layer + 0.5), delta_rayleigh, delta_mie );
	scattering = vec4( luminance_from_radiance * delta_rayleigh.rgb, (luminance_from_radiance * delta_mie).r );
	single_mie_scattering = luminance_from_radiance * delta_mie;
}

