
// Original by F. Strugar.

#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec3 position;
layout( binding = 0 ) uniform sampler2D g_terrainHeightmap;

uniform mat4 projViewMatrix;
uniform mat4 viewMatrix;

// .xy max used to clamp triangles outside of horizontal world range
uniform vec2 g_quadWorldMax;
// .x = gridDim, .y = gridDimHalf, .z = oneOverGridDimHalf
uniform vec3 g_gridDim;
// Size x/y/z and min x/y/z of terrain heightmap in world coords
uniform vec3 g_terrainScale;
uniform vec3 g_terrainOffset;
uniform vec2 g_samplerWorldToTextureScale;
uniform vec4 g_heightmapTextureInfo;

// x and z hold horizontal minimums, .y holds the y center of the bounding box
uniform vec3 g_nodeOffset;
// x and z holde the horizontal scale, y .w holds the current LOD level
uniform vec4 g_nodeScale;
uniform vec4 g_morphConsts;

//uniform mat4 g_viewProjection;
uniform vec3 g_cameraPos;
uniform vec3 g_diffuseLightDir;
uniform mat4 g_shadowMatrix;

out VERTEX_OUTPUT {
	vec4 position;
	vec2 globalUV;
	vec3 lightDir;	// .xyz
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
	float lightFactor;
	vec3 normal;
	float morphLerpK;
} vertOut;

const float HEIGHT_FACTOR = 655.35f;
const bool USE_MANUAL_FILTER = false;

// returns baseVertexPos where: xz are true values, y is center of the bounding box
vec3 getBaseVertexPos( vec3 inPosition ) {
	vec3 returnValue = inPosition * g_nodeScale.xyz + g_nodeOffset;
	returnValue.xz = min( returnValue.xz, g_quadWorldMax );
	return returnValue;
}

// Calculate texture world coordinates
vec2 calculateGlobalUV( vec2 vertex ) {
	vec2 globalUV = ( vertex.xy - g_terrainOffset.xz ) / g_terrainScale.xz;
	globalUV *= g_samplerWorldToTextureScale;
	globalUV += g_heightmapTextureInfo.zw * 0.5f;
	return globalUV;
}

//morphs vertex .xy from high to low detailed mesh position
vec2 morphVertex( vec3 inPosition, vec2 vertex, float morphLerpValue ) {
	vec2 decimals = ( fract( inPosition.xz * vec2( g_gridDim.y, g_gridDim.y ) ) * 
					vec2( g_gridDim.z, g_gridDim.z ) ) * g_nodeScale.xz;
	return vertex - decimals * morphLerpValue;
}

float sampleHeightmap( vec2 uv, bool useFilter = USE_MANUAL_FILTER ) {	
	if( useFilter ) {
		// Use filter in case of usampler2d to avoid discrete steps, ziggurat like hills
		vec2 heightmapSize = g_heightmapTextureInfo.xy;
		vec2 texelSize = g_heightmapTextureInfo.zw;
		uv = uv * heightmapSize - vec2( 0.5f, 0.5f );
		vec2 uvf = floor( uv );
		vec2 f = uv - uvf;
		uv = ( uvf + vec2( 0.5f, 0.5f ) ) * texelSize;
		float t00 = texture( g_terrainHeightmap, uv ).r;
		float t10 = texture( g_terrainHeightmap, vec2( uv.x + texelSize.x, uv.y ) ).r;
		float tA = mix( t00, t10, f.x );
		float t01 = texture( g_terrainHeightmap, vec2( uv.x, uv.y + texelSize.y ) ).r;
		float t11 = texture( g_terrainHeightmap, vec2( uv.x + texelSize.x, uv.y + texelSize.y ) ).r;
		float tB = mix( t01, t11, f.x );
		return mix( tA, tB, f.y ) * HEIGHT_FACTOR;
	} else
		// Assumes linear filtering being enabled in sampler
		return texture( g_terrainHeightmap, uv ).r * HEIGHT_FACTOR;
}

vec3 calculateNormal( vec2 uv ) {
	vec2 texelSize = g_heightmapTextureInfo.zw;
	float n = sampleHeightmap( uv + vec2( 0.0f, -texelSize.x ), USE_MANUAL_FILTER );
	float s = sampleHeightmap( uv + vec2( 0.0f, texelSize.x ), USE_MANUAL_FILTER );
	float e = sampleHeightmap( uv + vec2( -texelSize.y, 0.0f ), USE_MANUAL_FILTER );
	float w = sampleHeightmap( uv + vec2( texelSize.y, 0.0f ), USE_MANUAL_FILTER );
	vec3 sn = vec3( 0.0f , s - n, -( texelSize.y * 2.0f ) );
	vec3 ew = vec3( -( texelSize.x * 2.0f ), e - w, 0.0f );
	sn *= ( texelSize.y * 2.0f );
    ew *= ( texelSize.x * 2.0f );
    sn = normalize( sn );
    ew = normalize( ew );
    vec3 result = normalize( cross( sn, ew ) );
	return result;
}

void main() {
	// Pre-sample height to be able to precisely calculate morphing value.	
	vec3 vertex = getBaseVertexPos( position );
	//vertex.y = g_terrainOffset.y;
	
	vec2 preGlobalUV = calculateGlobalUV( vertex.xz );
	vertex.y = sampleHeightmap( preGlobalUV, USE_MANUAL_FILTER );
	//vertex.y = sampleHeightmap( preGlobalUV, USE_MANUAL_FILTER ) * g_terrainScale.z + g_terrainOffset.z;
	float eyeDistance = distance( vertex, g_cameraPos );
 	
	vertOut.morphLerpK = 1.0f - clamp( g_morphConsts.z - eyeDistance * g_morphConsts.w, 0.0f, 1.0f );
	vertex.xz = morphVertex( position, vertex.xz, vertOut.morphLerpK );
	
	vertOut.globalUV = calculateGlobalUV( vertex.xz );
	vertex.y = sampleHeightmap( vertOut.globalUV, USE_MANUAL_FILTER );
	//vertex.y = sampleHeightmap( vertOut.globalUV, USE_MANUAL_FILTER ) * g_terrainScale.z + g_terrainOffset.z;
	vertOut.position = projViewMatrix * vec4( vertex, 1.0f );
	
	vec3 normal = calculateNormal( vertOut.globalUV );
	vertOut.normal = normalize( normal * g_terrainScale.xyz );
	vertOut.lightDir = g_diffuseLightDir;
	vertOut.eyeDir = vec4( vertOut.position.xyz - g_cameraPos, eyeDistance );
	vertOut.lightFactor = clamp( dot( normal, g_diffuseLightDir ), 0.0f, 1.0f );
	
	gl_Position = vertOut.position;
}
