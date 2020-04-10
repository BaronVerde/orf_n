
#version 450 core

layout(location = 0) out vec3 delta_irradiance;
layout(location = 1) out vec3 irradiance;

uniform mat3 luminance_from_radiance;
uniform sampler3D single_rayleigh_scattering_texture;
uniform sampler3D single_mie_scattering_texture;
uniform sampler3D multiple_scattering_texture;
uniform int scattering_order;

const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;
const int SCATTERING_TEXTURE_R_SIZE = 32;
const int SCATTERING_TEXTURE_MU_SIZE = 128;
const int SCATTERING_TEXTURE_MU_S_SIZE = 32;
const int SCATTERING_TEXTURE_NU_SIZE = 8;
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

float RayleighPhaseFunction( float nu ) {
	float k = 3.0f / (16.0f * PI * sr);
	return k * (1.0f + nu * nu);
}

float MiePhaseFunction( float g, float nu ) {
	float k = 3.0f / (8.0f * PI * sr) * (1.0f - g * g) / (2.0f + g * g);
	return k * (1.0f + nu * nu) / pow(1.0f + g * g - 2.0 * g * nu, 1.5f);
}

vec4 GetScatteringTextureUvwzFromRMuMuSNu( float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground ) {
	float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
	float rho = SafeSqrt( r * r - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float u_r = GetTextureCoordFromUnitRange( rho / H, SCATTERING_TEXTURE_R_SIZE );
	float r_mu = r * mu;
	float discriminant = r_mu * r_mu - r * r + atmosphere.bottom_radius * atmosphere.bottom_radius;
	float u_mu;
	if( ray_r_mu_intersects_ground ) {
		float d = -r_mu - SafeSqrt(discriminant);
		float d_min = r - atmosphere.bottom_radius;
		float d_max = rho;
		u_mu = 0.5f - 0.5f *
			GetTextureCoordFromUnitRange( d_max == d_min ? 0.0f : (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
	} else {
		float d = -r_mu + SafeSqrt(discriminant + H * H);
		float d_min = atmosphere.top_radius - r;
		float d_max = rho + H;
		u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange( (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2 );
	}
	float d = DistanceToTopAtmosphereBoundary( atmosphere.bottom_radius, mu_s );
	float d_min = atmosphere.top_radius - atmosphere.bottom_radius;
	float d_max = H;
	float a = (d - d_min) / (d_max - d_min);
	float A = -2.0f * atmosphere.mu_s_min * atmosphere.bottom_radius / (d_max - d_min);
	float u_mu_s = GetTextureCoordFromUnitRange( max(1.0f - a / A, 0.0f) / (1.0f + a), SCATTERING_TEXTURE_MU_S_SIZE );
	float u_nu = (nu + 1.0f) / 2.0f;
	return vec4( u_nu, u_mu_s, u_mu, u_r );
}

vec3 GetScattering( sampler3D scattering_texture, float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground ) {
	vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu( r, mu, mu_s, nu, ray_r_mu_intersects_ground );
	float tex_coord_x = uvwz.x * float(SCATTERING_TEXTURE_NU_SIZE - 1 );
	float tex_x = floor(tex_coord_x);
	float lerp = tex_coord_x - tex_x;
	vec3 uvw0 = vec3( (tex_x + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w );
	vec3 uvw1 = vec3( (tex_x + 1.0 + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w );
	return vec3( texture(scattering_texture, uvw0) * (1.0f - lerp) + texture(scattering_texture, uvw1) * lerp );
}

vec3 GetScattering( float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground, int scattering_order ) {
	if( scattering_order == 1 ) {
		vec3 rayleigh = GetScattering( single_rayleigh_scattering_texture, r, mu, mu_s, nu, ray_r_mu_intersects_ground );
		vec3 mie = GetScattering( single_mie_scattering_texture, r, mu, mu_s, nu, ray_r_mu_intersects_ground );
		return rayleigh * RayleighPhaseFunction(nu) + mie * MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
	} else
		return GetScattering( multiple_scattering_texture, r, mu, mu_s, nu, ray_r_mu_intersects_ground );
}

vec3 ComputeIndirectIrradiance( float r, float mu_s, int scattering_order) {
	const int SAMPLE_COUNT = 32;
	const float dphi = pi / float(SAMPLE_COUNT);
	const float dtheta = pi / float(SAMPLE_COUNT);
	vec3 result = vec3( 0.0f * watt_per_square_meter_per_nm );
	vec3 omega_s = vec3( sqrt(1.0f - mu_s * mu_s), 0.0f, mu_s );
	for( int j = 0; j < SAMPLE_COUNT / 2; ++j ) {
		float theta = ( float(j) + 0.5f ) * dtheta;
		for( int i = 0; i < 2 * SAMPLE_COUNT; ++i ) {
			float phi = (float(i) + 0.5) * dphi;
			vec3 omega = vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
			float domega = (dtheta / rad) * (dphi / rad) * sin(theta) * sr;
			float nu = dot(omega, omega_s);
			// false is ray_r_theta_intersects_ground
			result += GetScattering( r, omega.z, mu_s, nu, false , scattering_order) * omega.z * domega;
		}
	}
	return result;
}

void GetRMuSFromIrradianceTextureUv( in vec2 uv, out float r, out float mu_s ) {
	float x_mu_s = GetUnitRangeFromTextureCoord(uv.x, IRRADIANCE_TEXTURE_WIDTH);
	float x_r = GetUnitRangeFromTextureCoord(uv.y, IRRADIANCE_TEXTURE_HEIGHT);
	r = atmosphere.bottom_radius + x_r * (atmosphere.top_radius - atmosphere.bottom_radius);
	mu_s = ClampCosine( 2.0f * x_mu_s - 1.0f );
}

const vec2 IRRADIANCE_TEXTURE_SIZE = vec2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
vec3 ComputeIndirectIrradianceTexture( in vec2 frag_coord, int scattering_order ) {
	float r;
	float mu_s;
	GetRMuSFromIrradianceTextureUv( frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s );
	return ComputeIndirectIrradiance( r, mu_s, scattering_order );
}

void main() {
	delta_irradiance = ComputeIndirectIrradianceTexture( gl_FragCoord.xy, scattering_order );
	irradiance = luminance_from_radiance * delta_irradiance;
}

