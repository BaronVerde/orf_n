
/**
 * Originally by Strugar's "CDLOD", edit for OpenGL and adapted to own framework,
 * enhanced with double precision RTE from Ring/Cozzy "Virtual Globes" (dsfun90) and
 * mapping of world positions to an ellipsoid.
 */ 

#version 450 core

#pragma debug(on)

// Constants
const float c_pi = 3.14159265f;
const float c_oneOverPi = 1.0f / c_pi;
const float c_oneOverTwoPi = 0.5f / c_pi;
const float c_toRadians = 0.0174533f; // 0.017453292519943296

// This is the position in the grid mesh, not world position !
layout( location = 0 ) in vec3 position;

// Texture with height values 0..1 ( * 65535 for real world values) above reference ellipsoid
layout( binding = 0 ) uniform sampler2D g_tileHeightmap;

uniform float u_height_factor = 1.0f;

// use linear filter manually. Not necessary if heightmap sampler is GL_LINEAR
// uniform bool u_useLinearFilter = false;
// --- Tile specific data (set on in the render loop on tile switch) ---
// Ellipsoid
uniform vec3 u_oneOverRadiiSquared;
uniform vec3 u_radiiSquared;
// Lower left world cartesian coordinate of heightmap tile
uniform vec3 g_tileOffset;
// Size x/y/z of heightmap tile in number of posts and max height.
uniform vec3 g_tileScale;
// Max of terrain tile. Used to clamp triangles outside of horizontal texture range.
uniform vec2 g_tileMax;
// (width-1)/width, (height-1)/height. Width and height are the same.
uniform vec2 g_tileToTexture;
// width, height, 1/width, 1/height in number of posts
uniform vec4 g_heightmapTextureInfo;

// --- Node specific data. Set in the render loop for every npde ---
// @todo: these could be constants if all tiles are the same !
// .x = gridDim, .y = gridDimHalf, .z = oneOverGridDimHalf
uniform vec3 g_gridDim;
// x and z hold horizontal minimums, .y holds the y center of the bounding box
uniform vec3 g_nodeOffset;
// x and z hold the horizontal scale of the bb in world size, .w holds the current lod level
uniform vec4 g_nodeScale;
// distances for current lod level for begin and end of morphing
// @todo: These are static in the application for now. Make them dynamic.
uniform vec4 g_morphConsts;
uniform vec3 g_diffuseLightDir;
layout( location = 5 ) uniform vec3 u_cameraPositionHigh;
layout( location = 6 ) uniform vec3 u_cameraPositionLow;
layout( location = 15 ) uniform mat4 u_viewProjectionMatrix;
layout( location = 18 ) uniform mat4 u_mvpMatrixRTE;

out VERTEX_OUTPUT {
	vec4 position;
	vec2 heightmapUV;
	vec3 lightDir;	// .xyz
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
	float lightFactor;
	vec3 normal;
	float morphLerpK;
} vertOut;

// Returns normal of a position in world space
vec3 geodeticSurfaceNormal( const vec3 pos ) {
	return normalize( pos * u_oneOverRadiiSquared );
}

// Returns normal from latitude/longitude given in radians
vec3 geodeticSurfaceNormal( const float latitude, const float longitude ) {
	const float cosLatitude = cos(latitude);
	return vec3( cosLatitude * cos(longitude), cosLatitude * sin(longitude), sin(latitude) );
}

// Calculates cartesian position from lat/lon in radians
vec3 toCartesian( const float latitude, const float longitude ) {
	const vec3 n = geodeticSurfaceNormal( latitude, longitude );
	const vec3 k = u_radiiSquared * n;
	const float gamma = sqrt( (k.x * n.x) + (k.y * n.y) + (k.z * n.z) );
	// cartesian surface position
	const vec3 rSurface = k / gamma;
	return rSurface + ( /*height * */n );
}

// Returns position relative to current tile fur texture lookup. Y value unsued.
vec3 getTileVertexPos( vec3 inPosition ) {
	vec3 returnValue = inPosition * g_nodeScale.xyz + g_nodeOffset;
	returnValue.xz = min( returnValue.xz, g_tileMax );
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

// Assumes linear filtering being enabled in sampler
float sampleHeightmap( vec2 uv ) {
	return texture( g_tileHeightmap, uv ).r * 655.35f;
}

// calculate vertex normal via central difference
vec3 calculateNormal( vec2 uv ) {
	vec2 texelSize = g_heightmapTextureInfo.zw;
	float n = sampleHeightmap( uv + vec2( 0.0f, -texelSize.x ) );
	float s = sampleHeightmap( uv + vec2( 0.0f, texelSize.x ) );
	float e = sampleHeightmap( uv + vec2( -texelSize.y, 0.0f ) );
	float w = sampleHeightmap( uv + vec2( texelSize.y, 0.0f ) );
	vec3 sn = vec3( 0.0f , s - n, -( texelSize.y * 2.0f ) );
	vec3 ew = vec3( -( texelSize.x * 2.0f ), e - w, 0.0f );
	sn *= ( texelSize.y * 2.0f );
    ew *= ( texelSize.x * 2.0f );
    sn = normalize( sn );
    ew = normalize( ew );
    vec3 result = normalize( cross( sn, ew ) );
	return result;
}

/* Calculate position rte from world position parts high/low and deduct camera position high/low.
 * Moves the world position into camera space. Result is single precision float */
vec3 calculatePositionRTE( vec3 posHigh, vec3 posLow ) {
	vec3 t1 = posLow - u_cameraPositionLow;
	vec3 e = t1 - posLow;
	vec3 t2 = ( ( -u_cameraPositionLow - e ) + ( posLow - ( t1 - e ) ) ) + posHigh - u_cameraPositionHigh;
	vec3 highDifference = t1 + t2;
	vec3 lowDifference = t2 - ( highDifference - t1 );
	return highDifference + lowDifference;
}

void main() {
	// calculate position on the heightmap for height value lookup
	vec3 vertex = getTileVertexPos( position );

	// Pre-sample height to be able to precisely calculate morphing value.
	vec2 preUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( preUV ) * u_height_factor;
	float eyeDistance = distance( vertex, u_cameraPositionHigh );

	vertOut.morphLerpK = 1.0f - clamp( g_morphConsts.z - eyeDistance * g_morphConsts.w, 0.0f, 1.0f );
	vertex.xz = morphVertex( position, vertex.xz, vertOut.morphLerpK );

	vertOut.heightmapUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( vertOut.heightmapUV ) * u_height_factor;

	// calculate position in world coordinates with the formula:
	// world position = tileOffset + tileVertexPosition * cellsize

	vertOut.position = u_viewProjectionMatrix * vec4( vertex, 1.0f );

	vec3 normal = calculateNormal( vertOut.heightmapUV );
	vertOut.normal = normalize( normal * g_tileScale.xyz );
	vertOut.lightDir = g_diffuseLightDir;
	vertOut.eyeDir = vec4( vertOut.position.xyz - u_cameraPositionHigh, eyeDistance );
	vertOut.lightFactor = clamp( dot( normal, g_diffuseLightDir ), 0.0f, 1.0f );

	gl_Position = vertOut.position;
}
