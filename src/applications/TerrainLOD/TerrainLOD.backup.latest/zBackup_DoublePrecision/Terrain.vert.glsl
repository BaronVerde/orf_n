
/**
 * Originally by Strugar's "CDLOD", edit for OpenGL and adapted to own framework,
 * enhanced with double precision RTE from Ring/Cozzy "Virtual Globes"
 */ 

#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec3 positionHigh;
layout( location = 1 ) in vec3 positionLow;
layout( binding = 0 ) uniform sampler2D g_tileHeightmap;

const float c_pi = 3.14159265f;
const float c_oneOverPi = 1.0f / c_pi;
const float c_oneOverTwoPi = 0.5f / c_pi;

// .xz used to clamp triangles outside of horizontal world range
uniform vec2 g_tileWorldMax;
// .x = gridDim, .y = gridDimHalf, .z = oneOverGridDimHalf
uniform vec3 g_gridDim;
// Space between heightmap posts in arcseconds 
uniform float cellsize;
// Size x/y/z of heightmap tile in number of posts. Multiply with cellsize to obtain world position
uniform vec3 g_tileScale;
// lower left coordinate of heightmap in degrees.decimals
uniform vec3 g_tileOffset;
uniform vec2 g_tileToTexture;
uniform vec4 g_heightmapTextureInfo;

// x and z hold horizontal minimums, .y holds the y center of the bounding box
uniform vec3 g_nodeOffset;
// x and z hold the horizontal scale, y .w holds the current LOD level
uniform vec4 g_nodeScale;
uniform vec4 g_morphConsts;

uniform mat4 projViewMatrix;
layout( location = 5 ) uniform vec3 u_cameraPositionHigh;
layout( location = 6 ) uniform vec3 u_cameraPositionLow;
uniform vec3 g_diffuseLightDir;

uniform float u_heightFactor = 655.35f;
// use linear filter manually. Not necessary if heightmap sampler is GL_LINEAR
uniform bool u_useLinearFilter = false;
uniform vec3 u_oneOverRadiiSquared;

out VERTEX_OUTPUT {
	vec4 position;
	vec2 heightmapUV;
	vec3 lightDir;	// .xyz
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
	float lightFactor;
	vec3 normal;
	float morphLerpK;
} vertOut;

// of a position in rte space
vec3 geodeticSurfaceNormal( const vec3 pos ) {
	return normalize( pos * u_oneOverRadiiSquared );
}

// returns position relative to current tile fur texture lookup where:
// xz are true values, y is center of the tile-relative bounding box
vec3 getTileVertexPos( vec3 inPosition ) {
	vec3 returnValue = inPosition * g_nodeScale.xyz + g_nodeOffset;
	returnValue.xz = min( returnValue.xz, g_tileWorldMax );	
	return returnValue;
}

// Calculate texture coordinates for the heightmap. Observe lod node's offset and scale.
vec2 calculateUV( vec2 vertex ) {
	vec2 heightmapUV = ( vertex.xy - g_tileOffset.xz ) / g_tileScale.xz;
	heightmapUV *= g_tileToTexture;
	heightmapUV += g_heightmapTextureInfo.zw * 0.5f;
	return heightmapUV;
}

// morphs vertex .xy from high to low detailed mesh position
vec2 morphVertex( vec3 inPosition, vec2 vertex, float morphLerpValue ) {
	vec2 decimals = ( fract( inPosition.xz * vec2( g_gridDim.y, g_gridDim.y ) ) * 
					vec2( g_gridDim.z, g_gridDim.z ) ) * g_nodeScale.xz;
	return vertex - decimals * morphLerpValue;
}

float sampleHeightmap( vec2 uv, bool useFilter ) {	
	if( useFilter ) {
		// Use filter in case of usampler2d to avoid discrete steps, ziggurat like hills
		vec2 heightmapSize = g_heightmapTextureInfo.xy;
		vec2 texelSize = g_heightmapTextureInfo.zw;
		uv = uv * heightmapSize - vec2( 0.5f, 0.5f );
		vec2 uvf = floor( uv );
		vec2 f = uv - uvf;
		uv = ( uvf + vec2( 0.5f, 0.5f ) ) * texelSize;
		float t00 = texture( g_tileHeightmap, uv ).r;
		float t10 = texture( g_tileHeightmap, vec2( uv.x + texelSize.x, uv.y ) ).r;
		float tA = mix( t00, t10, f.x );
		float t01 = texture( g_tileHeightmap, vec2( uv.x, uv.y + texelSize.y ) ).r;
		float t11 = texture( g_tileHeightmap, vec2( uv.x + texelSize.x, uv.y + texelSize.y ) ).r;
		float tB = mix( t01, t11, f.x );
		return mix( tA, tB, f.y ) * u_heightFactor;
	} else
		// Assumes linear filtering being enabled in sampler
		return texture( g_tileHeightmap, uv ).r * u_heightFactor;
}

// calculate normal at uv by central difference 
vec3 calculateNormal( vec2 uv ) {
	vec2 texelSize = g_heightmapTextureInfo.zw;
	float n = sampleHeightmap( uv + vec2( 0.0f, -texelSize.x ), false );
	float s = sampleHeightmap( uv + vec2( 0.0f, texelSize.x ), false );
	float e = sampleHeightmap( uv + vec2( -texelSize.y, 0.0f ), false );
	float w = sampleHeightmap( uv + vec2( texelSize.y, 0.0f ), false );
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
	// calculate position on the heightmap for height value lookup
	vec3 vertex = getTileVertexPos( positionHigh );
	
	// Pre-sample height to be able to precisely calculate morphing value.
	vec2 preUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( preUV, u_useLinearFilter );	
	float eyeDistance = distance( vertex, u_cameraPositionHigh );
 	
	vertOut.morphLerpK = 1.0f - clamp( g_morphConsts.z - eyeDistance * g_morphConsts.w, 0.0f, 1.0f );
	vertex.xz = morphVertex( positionHigh, vertex.xz, vertOut.morphLerpK );
	
	vertOut.heightmapUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( vertOut.heightmapUV, u_useLinearFilter );
	
	// calculate position in world coordinates with the formula:	
	// world position = tileOffset + tileVertexPosition * cellsize  
		
	vertOut.position = projViewMatrix * vec4( vertex, 1.0f );
	
	vec3 normal = calculateNormal( vertOut.heightmapUV );
	vertOut.normal = normalize( normal * g_tileScale.xyz );
	vertOut.lightDir = g_diffuseLightDir;
	vertOut.eyeDir = vec4( vertOut.position.xyz - u_cameraPositionHigh, eyeDistance );
	vertOut.lightFactor = clamp( dot( normal, g_diffuseLightDir ), 0.0f, 1.0f );
	
	gl_Position = vertOut.position;
}
