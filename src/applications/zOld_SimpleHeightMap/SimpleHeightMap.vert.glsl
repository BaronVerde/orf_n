
#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec2 flatPosition;

layout( location = 5 ) uniform vec3 og_cameraPosition;
layout( location = 9 ) uniform mat4 og_modelViewPerspectiveMatrix;
layout( location = 11 ) uniform vec3 og_sunPosition;

layout( binding = 0 ) uniform sampler2D og_heightMap;

uniform float u_heightExaggeration;
// should always be rectangular, check in HeightMap.cpp !
uniform vec2 u_extent;
uniform vec2 u_positionToTextureCoordinate;
uniform vec2 u_positionToRepeatTextureCoordinate;
uniform float u_sobelFactor;
uniform int u_normalAlgorithm;

out VS_DATA {
	vec3 normal;
	vec3 positionToLight;
	vec3 positionToEye;
	vec2 textureCoordinate;
	vec2 repeatTextureCoordinate;
	float height;
} vs_out;

// Intake is displaced position
vec3 computeNormalForwardDifference( vec3 pos ) {
	float heightRight = texture( og_heightMap, 
		vec2( pos.x + 1.0, pos.z ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
    vec3 right = vec3( pos.x + 1.0, heightRight, pos.z );
    float heightTop = texture( og_heightMap, 
    	vec2( pos.x, pos.z + 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
    vec3 top = vec3( pos.x, heightTop, pos.z + 1.0 );
    // Why are the normals upside down ?
    return -cross( right - pos, top - pos );
}

// Intake is displaced position
vec3 computeNormalCentralDifference( vec3 pos ) {
	float leftHeight = texture( og_heightMap,
		vec2( pos.x - 1.0, pos.z ) * u_positionToTextureCoordinate).r * u_heightExaggeration;
    vec3 left = vec3( pos.x - 1.0, leftHeight, pos.z );
	float rightHeight = texture( og_heightMap,
		vec2( pos.x + 1.0, pos.z ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
    vec3 right = vec3( pos.x + 1.0, rightHeight, pos.z );
	float bottomHeight = texture( og_heightMap, 
		vec2( pos.x, pos.z - 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
    vec3 bottom = vec3( pos.x, bottomHeight, pos.z - 1.0 );
    float topHeight = texture( og_heightMap, 
    	vec2( pos.x, pos.z + 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
    vec3 top = vec3( pos.x, topHeight, pos.z + 1.0 );
    // Why are the normals upside down ?
    return -cross( right - left, top - bottom );
}

/* float sumElements( mat3 m ) {
    return m[0].x + m[0].y + m[0].z +
           m[1].x + m[1].y + m[1].z +
	       m[2].x + m[2].y + m[2].z;
} */

// Intake is displaced position
vec3 computeNormalSobelFilter( vec3 pos ) {
	float upperLeft = texture( og_heightMap, 
		vec2( pos.x - 1.0, pos.z + 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	float upperCenter = texture( og_heightMap, 
		vec2( pos.x, pos.z + 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	float upperRight = texture( og_heightMap,
		vec2( pos.x + 1.0, pos.z + 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	float left = texture( og_heightMap,
		vec2( pos.x - -1.0, pos.z ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	float right = texture( og_heightMap,
		vec2( pos.x + 1.0, pos.z ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	float lowerLeft = texture( og_heightMap,
		vec2( pos.x - 1.0, pos.z - 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	float lowerCenter = texture( og_heightMap,
		vec2( pos.x, pos.z - 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	float lowerRight = texture( og_heightMap,
		vec2( pos.x + 1.0, pos.z - 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
	
	/* mat3 positions = mat3( upperLeft, left, lowerLeft,
	    				   upperCenter, 0.0, lowerCenter,
						   upperRight, right, lowerRight );
	mat3 sobelX = mat3( -1.0, -2.0, -1.0,
					     0.0,  0.0,  0.0,
					     1.0,  2.0,  1.0 );
	mat3 sobelY = mat3( -1.0, 0.0, 1.0,
	    				-2.0, 0.0, 2.0,
					    -1.0, 0.0, 1.0 );					    
	float x = sumElements( matrixCompMult( positions, sobelX ) );
	float y = sumElements( matrixCompMult( positions, sobelY ) );*/
	
	float x = upperRight + ( 2.0 * right ) + lowerRight - upperLeft - ( 2.0 * left ) - lowerLeft;
    float y = lowerLeft + ( 2.0 * lowerCenter ) + lowerRight - upperLeft - ( 2.0 * upperCenter ) - upperRight;
    
    // These normals are correct side up ...
	return vec3( -x, u_sobelFactor, y );
}

void main() {
	vs_out.height = texture( og_heightMap, flatPosition * u_positionToTextureCoordinate ).r;
	vec3 displacedPosition = vec3( flatPosition.x, vs_out.height * u_heightExaggeration, flatPosition.y );
	
	gl_Position = og_modelViewPerspectiveMatrix * vec4( displacedPosition, 1.0 );
	
	if( 1 == u_normalAlgorithm )
		vs_out.normal = computeNormalForwardDifference( displacedPosition );
	else if( 2 == u_normalAlgorithm )
		vs_out.normal = computeNormalCentralDifference( displacedPosition );
	else if( 3 == u_normalAlgorithm )
		vs_out.normal = computeNormalSobelFilter( displacedPosition );
		
	vs_out.positionToLight = og_sunPosition - displacedPosition;
    vs_out.positionToEye = og_cameraPosition - displacedPosition;
	vs_out.textureCoordinate = flatPosition * u_positionToTextureCoordinate;
 	vs_out.repeatTextureCoordinate = flatPosition * u_positionToRepeatTextureCoordinate;
}

