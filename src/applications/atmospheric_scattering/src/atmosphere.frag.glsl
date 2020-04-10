
#version 450 core

const float kPi = 3.14159265f;
const float m = 1.0f;
const float nm = 1.0f;
const float m2 = m * m;
const float sr = 1.0f;
const float rad = 1.0f;
const float watt = 1.0f;
const float watt_per_square_meter_per_sr_per_nm = watt / ( m2 * sr * nm );
const float kLengthUnitInMeters = 1000.0f;	// 1 km
const vec3 kSphereCenter = vec3( 0.0f, 0.0f, 1000.0f ) / kLengthUnitInMeters;
const float kSphereRadius = 1000.0f / kLengthUnitInMeters;
const vec3 kSphereAlbedo = vec3( 0.8f );
const vec3 kGroundAlbedo = vec3( 0.0f, 0.0f, 0.04f );
const vec3 kSkySpectralRadianceToLuminance = vec3( 114974.916437f, 71305.954816f, 65310.548555f );
const vec3 kSunSpectralRadianceToLuminance = vec3( 98242.786222f, 69954.398112f, 66475.012354f );
const int kTransmittanceTextureWidth = 256;
const int kTransmittanceTextureHeight = 64;
const int kIrradianceTextureWidth = 64;
const int kIrradianceTextureHeight = 16;
const int kScatteringTextureRSize = 32;
const int kScatteringTextureMUSize = 128;
const int kScatteringTextureMU_SSize = 32;
const int kScatteringTextureNUSize = 8;
const int kIrradianceTextureSize = kIrradianceTextureWidth * kIrradianceTextureHeight;

in vec3 view_ray;

uniform vec3 camera;
uniform vec3 earth_center;
uniform vec3 sun_direction;
uniform vec2 sun_size;
uniform float exposure;
uniform vec3 white_point;
// @todo default: 0 = none; use correctly
uniform bool use_luminance = false;

uniform sampler2D transmittance_texture;
uniform sampler2D irradiance_texture;
uniform sampler3D scattering_texture;
uniform sampler3D single_mie_scattering_texture;

layout( location = 0 ) out vec4 color;

// @todo these will be uniforms as well
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

// @todo this is a static uniform buffer
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

// @todo: this was handed down !
const AtmosphereParameters atmosphere = AtmosphereParameters( 
	vec3( 1.474f,1.8504f,1.91198f ),
	0.004675, 6360.0f, 6420.0f,
	DensityProfile(
		DensityProfileLayer[2] (
			DensityProfileLayer( 0.0f,0.0f,0.0f,0.0f,0.0f ), DensityProfileLayer( 0.0f,1.0f,-0.125f,0.0f,0.0f )
		)
	),
	vec3( 0.005802f, 0.013558f, 0.0331f ),
	DensityProfile(
		DensityProfileLayer[2] (
			DensityProfileLayer( 0.0f,0.0f,0.0f,0.0f,0.0f ), DensityProfileLayer( 0.0f,1.0f,-0.833333f,0.0f,0.0f )
		)
	),
	vec3(0.003996f,0.003996f,0.003996f),
	vec3(0.00444f,0.00444f,0.00444f),
	0.8f,
	DensityProfile(
		DensityProfileLayer[2] (
			DensityProfileLayer(25.0f,0.0f,0.0f,0.066667f,-0.666667f ), DensityProfileLayer(0.0f,0.0f,0.0f,-0.066667f,2.666667f)
		)
	),
	vec3(0.00065f,0.001881f,0.000085f),
	vec3(0.1f,0.1f,0.1f),
	-0.207912f
);

float SafeSqrt( float a ) {
	return sqrt( max( a, 0.0f * m2 ) );
}

float ClampCosine( float mu ) {
	return clamp( mu, -1.0f, 1.0f );
}

float ClampDistance( float d ) {
	return max( d, 0.0f * m );
}

float ClampRadius( float r ) {
	return clamp( r, atmosphere.bottom_radius, atmosphere.top_radius );
}

bool RayIntersectsGround( float r, float mu ) {
	return mu < 0.0f && r * r * ( mu * mu - 1.0f ) + atmosphere.bottom_radius * atmosphere.bottom_radius >= 0.0f * m2;
}

float DistanceToTopAtmosphereBoundary( float r, float mu ) {
	float discriminant = r * r * ( mu * mu - 1.0f ) + atmosphere.top_radius * atmosphere.top_radius;
	return ClampDistance( -r * mu + SafeSqrt( discriminant ) );
}

float GetTextureCoordFromUnitRange( float x, int texture_size ) {
	return 0.5f / float( texture_size ) + x * ( 1.0f - 1.0f / float( texture_size ) );
}

float RayleighPhaseFunction( float nu ) {
	float k = 3.0f / ( 16.0f * kPi * sr );
	return k * ( 1.0f + nu * nu );
}

float MiePhaseFunction( float g, float nu ) {
	float k = 3.0f / ( 8.0f * kPi * sr ) * ( 1.0f - g * g) / ( 2.0f + g * g );
	return k * ( 1.0f + nu * nu ) / pow( 1.0f + g * g - 2.0f * g * nu, 1.5f );
}

float GetSunVisibility( vec3 point ) {
	vec3 p = point - kSphereCenter;
	float p_dot_v = dot( p, sun_direction );
	float p_dot_p = dot( p, p );
	float ray_sphere_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
	float distance_to_intersection = -p_dot_v - sqrt( kSphereRadius * kSphereRadius - ray_sphere_center_squared_distance );
	if (distance_to_intersection > 0.0) {
		float ray_sphere_distance = kSphereRadius - sqrt( ray_sphere_center_squared_distance );
		float ray_sphere_angular_distance = -ray_sphere_distance / p_dot_v;
		return smoothstep( 1.0f, 0.0f, ray_sphere_angular_distance / sun_size.x );
	}
	return 1.0;
}

float GetSkyVisibility( vec3 point ) {
	vec3 p = point - kSphereCenter;
	float p_dot_p = dot( p, p );
	return 1.0f + p.z / sqrt( p_dot_p ) * kSphereRadius * kSphereRadius / p_dot_p;
}

vec2 GetIrradianceTextureUvFromRMuS( float r, float mu_s ) {
	float x_r = ( r - atmosphere.bottom_radius ) / ( atmosphere.top_radius - atmosphere.bottom_radius );
	float x_mu_s = mu_s * 0.5f + 0.5f;
	return vec2( GetTextureCoordFromUnitRange( x_mu_s, kIrradianceTextureWidth ),
				 GetTextureCoordFromUnitRange( x_r, kIrradianceTextureHeight ) );
}

vec3 GetIrradiance( float r, float mu_s ) {
	vec2 uv = GetIrradianceTextureUvFromRMuS( r, mu_s );
	return vec3( texture( irradiance_texture, uv ) );
}

vec2 GetTransmittanceTextureUvFromRMu( float r, float mu) {
	float H = sqrt( atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float rho = SafeSqrt( r * r - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float d = DistanceToTopAtmosphereBoundary( r, mu );
	float d_min = atmosphere.top_radius - r;
	float d_max = rho + H;
	float x_mu = ( d - d_min ) / ( d_max - d_min );
	float x_r = rho / H;
	return vec2( GetTextureCoordFromUnitRange( x_mu, kTransmittanceTextureWidth ),
				 GetTextureCoordFromUnitRange( x_r, kTransmittanceTextureHeight ) );
}

vec3 GetTransmittanceToTopAtmosphereBoundary( float r, float mu) {
	vec2 uv = GetTransmittanceTextureUvFromRMu( r, mu );
	return vec3( texture( transmittance_texture, uv ) );
}

vec3 GetTransmittanceToSun( float r, float mu_s ) {
	float sin_theta_h = atmosphere.bottom_radius / r;
	float cos_theta_h = -sqrt( max( 1.0f - sin_theta_h * sin_theta_h, 0.0f ) );
	return GetTransmittanceToTopAtmosphereBoundary( r, mu_s ) * 
			smoothstep( -sin_theta_h * atmosphere.sun_angular_radius / rad,
						sin_theta_h * atmosphere.sun_angular_radius / rad,
						mu_s - cos_theta_h );
}

vec3 GetTransmittance( float r, float mu, float d, bool ray_r_mu_intersects_ground ) {
	float r_d = ClampRadius( sqrt( d * d + 2.0f * r * mu * d + r * r ) );
	float mu_d = ClampCosine( ( r * mu + d ) / r_d );
	if( ray_r_mu_intersects_ground ) {
		return min(
			GetTransmittanceToTopAtmosphereBoundary( r_d, -mu_d ) /
			GetTransmittanceToTopAtmosphereBoundary( r, -mu ),
			vec3( 1.0f ) );
	} else {
		return min(
			GetTransmittanceToTopAtmosphereBoundary( r, mu ) /
			GetTransmittanceToTopAtmosphereBoundary( r_d, mu_d ),
			vec3( 1.0f ) );
	}
}

vec3 GetSunAndSkyIrradiance( const in vec3 point, const in vec3 normal, out vec3 sky_irradiance ) {
	float r = length( point );
	float mu_s = dot( point, sun_direction ) / r;
	sky_irradiance = GetIrradiance( r, mu_s ) * ( 1.0f + dot( normal, point ) / r ) * 0.5f;
	return atmosphere.solar_irradiance * GetTransmittanceToSun( r, mu_s ) * max( dot( normal, sun_direction ), 0.0f );
}

vec3 GetSunAndSkyIlluminance( vec3 p, vec3 normal, out vec3 sky_irradiance ) {
	vec3 sun_irradiance = GetSunAndSkyIrradiance( p, normal, sky_irradiance );
	sky_irradiance *= kSkySpectralRadianceToLuminance;
	return sun_irradiance * kSunSpectralRadianceToLuminance;
}

void GetSphereShadowInOut( vec3 view_direction, out float d_in, out float d_out ) {
	vec3 pos = camera - kSphereCenter;
	float pos_dot_sun = dot( pos, sun_direction );
	float view_dot_sun = dot( view_direction, sun_direction );
	float k = sun_size.x;
	float l = 1.0f + k * k;
	float a = 1.0f - l * view_dot_sun * view_dot_sun;
	float b = dot( pos, view_direction ) - l * pos_dot_sun * view_dot_sun - k * kSphereRadius * view_dot_sun;
	float c = dot( pos, pos ) - l * pos_dot_sun * pos_dot_sun - 
		2.0f * k * kSphereRadius * pos_dot_sun - kSphereRadius * kSphereRadius;
	float discriminant = b * b - a * c;
	if( discriminant > 0.0f ) {
		d_in = max( 0.0f, ( -b - sqrt(discriminant) ) / a );
		d_out = ( -b + sqrt(discriminant) ) / a;
		float d_base = -pos_dot_sun / view_dot_sun;
		float d_apex = -( pos_dot_sun + kSphereRadius / k ) / view_dot_sun;
		if( view_dot_sun > 0.0f ) {
			d_in = max( d_in, d_apex );
			d_out = a > 0.0f ? min( d_out, d_base ) : d_base;
		} else {
			d_in = a > 0.0f ? max( d_in, d_base ) : d_base;
			d_out = min( d_out, d_apex );
		}
	} else {
		d_in = 0.0f;
		d_out = 0.0f;
	}
}

vec4 GetScatteringTextureUvwzFromRMuMuSNu( float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground) {
	float H = sqrt( atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float rho = SafeSqrt( r * r - atmosphere.bottom_radius * atmosphere.bottom_radius );
	float u_r = GetTextureCoordFromUnitRange( rho / H, kScatteringTextureRSize );
	float r_mu = r * mu;
	float discriminant = r_mu * r_mu - r * r + atmosphere.bottom_radius * atmosphere.bottom_radius;
	float u_mu;
	if( ray_r_mu_intersects_ground ) {
		float d = -r_mu - SafeSqrt( discriminant );
		float d_min = r - atmosphere.bottom_radius;
		float d_max = rho;
		u_mu = 0.5f - 0.5f * 
		GetTextureCoordFromUnitRange( d_max == d_min ? 0.0f : (d-d_min)/(d_max-d_min), kScatteringTextureMUSize / 2 );
	} else {
		float d = -r_mu + SafeSqrt( discriminant + H * H );
		float d_min = atmosphere.top_radius - r;
		float d_max = rho + H;
		u_mu = 0.5f + 0.5f *
		GetTextureCoordFromUnitRange( (d - d_min) / (d_max - d_min), kScatteringTextureMUSize / 2 );
	}
	float d = DistanceToTopAtmosphereBoundary( atmosphere.bottom_radius, mu_s );
	float d_min = atmosphere.top_radius - atmosphere.bottom_radius;
	float d_max = H;
	float a = (d - d_min) / (d_max - d_min);
	float A = -2.0f * atmosphere.mu_s_min * atmosphere.bottom_radius / (d_max - d_min);
	float u_mu_s = GetTextureCoordFromUnitRange( max( 1.0f - a / A, 0.0f ) / (1.0f + a), kScatteringTextureMU_SSize );
	float u_nu = (nu + 1.0f) / 2.0f;
	return vec4( u_nu, u_mu_s, u_mu, u_r );
}

vec3 GetCombinedScattering( float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground, 
		out vec3 single_mie_scattering) {
	vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu( r, mu, mu_s, nu, ray_r_mu_intersects_ground );
	float tex_coord_x = uvwz.x * float( kScatteringTextureNUSize - 1 );
	float tex_x = floor( tex_coord_x );
	float lerp = tex_coord_x - tex_x;
	vec3 uvw0 = vec3( ( tex_x + uvwz.y ) / float( kScatteringTextureNUSize ), uvwz.z, uvwz.w );
	vec3 uvw1 = vec3( ( tex_x + 1.0f + uvwz.y ) / float( kScatteringTextureNUSize ), uvwz.z, uvwz.w );
	vec3 scattering;
	scattering = vec3(
		texture( scattering_texture, uvw0 ) * (1.0f - lerp) + texture( scattering_texture, uvw1 ) * lerp
	);
	single_mie_scattering = vec3(
		texture( single_mie_scattering_texture, uvw0 ) * ( 1.0f - lerp ) +
		texture( single_mie_scattering_texture, uvw1 ) * lerp
	);
	return scattering;
}

vec3 GetSkyRadiance( vec3 camera, const in vec3 view_ray, float shadow_length, out vec3 transmittance ) {
	float r = length( camera );
	float rmu = dot( camera, view_ray );
	float distance_to_top_atmosphere_boundary =
		-rmu - sqrt( rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius );
	if( distance_to_top_atmosphere_boundary > 0.0f * m ) {
		camera = camera + view_ray * distance_to_top_atmosphere_boundary;
		r = atmosphere.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	} else if( r > atmosphere.top_radius ) {
		transmittance = vec3(1.0f);
		return vec3( 0.0f * watt_per_square_meter_per_sr_per_nm );
	}
	float mu = rmu / r;
	float mu_s = dot( camera, sun_direction ) / r;
	float nu = dot( view_ray, sun_direction );
	bool ray_r_mu_intersects_ground = RayIntersectsGround( r, mu );
	transmittance = ray_r_mu_intersects_ground ? vec3(0.0f) :
			GetTransmittanceToTopAtmosphereBoundary( r, mu );
	vec3 single_mie_scattering;
	vec3 scattering;
	if( shadow_length == 0.0f * m ) {
		scattering = GetCombinedScattering(r, mu, mu_s, nu, ray_r_mu_intersects_ground, single_mie_scattering );
	} else {
		float d = shadow_length;
		float r_p = ClampRadius( sqrt( d * d + 2.0f * r * mu * d + r * r ) );
		float mu_p = ( r * mu + d ) / r_p;
		float mu_s_p = ( r * mu_s + d * nu ) / r_p;
		scattering = GetCombinedScattering( r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground, single_mie_scattering );
		vec3 shadow_transmittance = GetTransmittance( r, mu, shadow_length, ray_r_mu_intersects_ground );
		scattering = scattering * shadow_transmittance;
		single_mie_scattering = single_mie_scattering * shadow_transmittance;
	}
	return scattering * RayleighPhaseFunction( nu ) + 
			single_mie_scattering * MiePhaseFunction( atmosphere.mie_phase_function_g, nu );
}

vec3 GetSkyRadianceToPoint( vec3 camera, const in vec3 point, float shadow_length, out vec3 transmittance ) {
	vec3 view_ray = normalize( point - camera );
	float r = length( camera );
	float rmu = dot( camera, view_ray );
	float distance_to_top_atmosphere_boundary = -rmu - 
		sqrt( rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius );
	if( distance_to_top_atmosphere_boundary > 0.0f * m ) {
		camera = camera + view_ray * distance_to_top_atmosphere_boundary;
		r = atmosphere.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	}
	float mu = rmu / r;
	float mu_s = dot( camera, sun_direction ) / r;
	float nu = dot( view_ray, sun_direction );
	float d = length( point - camera );
	bool ray_r_mu_intersects_ground = RayIntersectsGround( r, mu );
	transmittance = GetTransmittance( r, mu, d, ray_r_mu_intersects_ground );
	vec3 single_mie_scattering;
	vec3 scattering = GetCombinedScattering( r, mu, mu_s, nu, ray_r_mu_intersects_ground, single_mie_scattering );
	d = max( d - shadow_length, 0.0f * m );
	float r_p = ClampRadius( sqrt( d * d + 2.0f * r * mu * d + r * r ) );
	float mu_p = (r * mu + d) / r_p;
	float mu_s_p = (r * mu_s + d * nu) / r_p;
	vec3 single_mie_scattering_p;
	vec3 scattering_p = GetCombinedScattering( r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground, single_mie_scattering_p );
	vec3 shadow_transmittance = transmittance;
	if( shadow_length > 0.0f * m )
		shadow_transmittance = GetTransmittance( r, mu, d, ray_r_mu_intersects_ground );
	scattering = scattering - shadow_transmittance * scattering_p;
	// @todo if ... else ?
	single_mie_scattering = single_mie_scattering - shadow_transmittance * single_mie_scattering_p;
	single_mie_scattering = single_mie_scattering * smoothstep( 0.0f, 0.01f, mu_s );
	return scattering * RayleighPhaseFunction( nu ) + 
		single_mie_scattering * MiePhaseFunction( atmosphere.mie_phase_function_g, nu );
}

vec3 GetSkyLuminance( vec3 camera, vec3 view_ray, float shadow_length, out vec3 transmittance) {
	return GetSkyRadiance( camera, view_ray, shadow_length, transmittance ) *
		kSkySpectralRadianceToLuminance;
}

vec3 GetSkyLuminanceToPoint( vec3 camera, vec3 point, float shadow_length, out vec3 transmittance) {
	return GetSkyRadianceToPoint( camera, point, shadow_length, transmittance ) * kSkySpectralRadianceToLuminance;
}

vec3 GetSolarRadiance() {
	return atmosphere.solar_irradiance / ( kPi * atmosphere.sun_angular_radius * atmosphere.sun_angular_radius );
}

vec3 GetSolarLuminance() {
	return atmosphere.solar_irradiance / ( kPi * atmosphere.sun_angular_radius * atmosphere.sun_angular_radius) *
		kSunSpectralRadianceToLuminance;
}

void main() {
	vec3 view_direction = normalize( view_ray );
	float fragment_angular_size = length( dFdx( view_ray) + dFdy( view_ray ) ) / length( view_ray );
	float shadow_in;
	float shadow_out;
	GetSphereShadowInOut( view_direction, shadow_in, shadow_out );
	float lightshaft_fadein_hack = smoothstep( 0.02f, 0.04f, dot( normalize( camera - earth_center ), sun_direction ) );
	vec3 p = camera - kSphereCenter;
	float p_dot_v = dot( p, view_direction );
	float p_dot_p = dot( p, p );
	float ray_sphere_distance_sq = p_dot_p - p_dot_v * p_dot_v;
	float distance_to_intersection = -p_dot_v - sqrt( kSphereRadius * kSphereRadius - ray_sphere_distance_sq );
	float sphere_alpha = 0.0f;
	vec3 sphere_radiance = vec3( 0.0f );
	if( distance_to_intersection > 0.0f ) {
		float ray_sphere_distance = kSphereRadius - sqrt( ray_sphere_distance_sq );
		float ray_sphere_angular_distance = -ray_sphere_distance / p_dot_v;
		sphere_alpha = min( ray_sphere_angular_distance / fragment_angular_size, 1.0f );
		vec3 point = camera + view_direction * distance_to_intersection;
		vec3 normal = normalize( point - kSphereCenter );
		vec3 sky_irradiance;
		vec3 sun_irradiance = use_luminance ?
			GetSunAndSkyIrradiance( point - earth_center, normal, sky_irradiance ) :
			GetSunAndSkyIrradiance( point - earth_center, normal, sky_irradiance );
		sphere_radiance = kSphereAlbedo * ( 1.0f / kPi ) * ( sun_irradiance + sky_irradiance );
		float shadow_length = max( 0.0f, min( shadow_out, distance_to_intersection ) - shadow_in ) * lightshaft_fadein_hack;
		vec3 transmittance;
		vec3 in_scatter = use_luminance ? 
			GetSkyLuminanceToPoint( camera - earth_center, point - earth_center, shadow_length, transmittance ) :
			GetSkyRadianceToPoint( camera - earth_center, point - earth_center, shadow_length, transmittance );
		sphere_radiance = sphere_radiance * transmittance + in_scatter;
	}
	p = camera - earth_center;
	p_dot_v = dot( p, view_direction );
	p_dot_p = dot( p, p );
	float ray_earth_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
	distance_to_intersection = -p_dot_v - 
		sqrt( earth_center.z * earth_center.z - ray_earth_center_squared_distance );
	float ground_alpha = 0.0f;
	vec3 ground_radiance = vec3( 0.0f );
	if( distance_to_intersection > 0.0f ) {
		vec3 point = camera + view_direction * distance_to_intersection;
		vec3 normal = normalize( point - earth_center );
		vec3 sky_irradiance;
		vec3 sun_irradiance = use_luminance ?
			GetSunAndSkyIrradiance( point - earth_center, normal, sky_irradiance ) :
			GetSunAndSkyIrradiance( point - earth_center, normal, sky_irradiance );
		ground_radiance = kGroundAlbedo * (1.0f / kPi) *
			( sun_irradiance * GetSunVisibility( point ) + sky_irradiance * GetSkyVisibility( point ) );
		float shadow_length =
			max( 0.0f, min( shadow_out, distance_to_intersection ) - shadow_in ) * lightshaft_fadein_hack;
		vec3 transmittance;
		vec3 in_scatter = use_luminance ?
			GetSkyLuminanceToPoint( camera - earth_center, point - earth_center, shadow_length, transmittance ) :		
			GetSkyRadianceToPoint( camera - earth_center, point - earth_center, shadow_length, transmittance );
		ground_radiance = ground_radiance * transmittance + in_scatter;
		ground_alpha = 1.0f;
	}
	float shadow_length = max( 0.0f, shadow_out - shadow_in ) * lightshaft_fadein_hack;
	vec3 transmittance;
	vec3 radiance = use_luminance ?
		GetSkyLuminance( camera - earth_center, view_direction, shadow_length, transmittance ) :
		GetSkyRadiance( camera - earth_center, view_direction, shadow_length, transmittance );
	if( dot( view_direction, sun_direction ) > sun_size.y )
		radiance = radiance + transmittance * ( use_luminance ? GetSolarLuminance() : GetSolarRadiance() );
	radiance = mix( radiance, ground_radiance, ground_alpha );
	radiance = mix( radiance, sphere_radiance, sphere_alpha );
	color.rgb = pow( vec3(1.0f) - exp( -radiance / white_point * exposure ), vec3( 1.0f / 2.2f ) );
	color.a = 1.0f;
}

