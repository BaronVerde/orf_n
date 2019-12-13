
﻿#version 450 core

layout( points ) in;
layout( triangle_strip, max_vertices = 4 ) out;

in float gsDistanceToEye[];
out float fsDistanceToEye;

layout( location = 9 ) uniform mat4 og_modelViewPerspectiveMatrix;
uniform mat4 og_viewportTransformationMatrix;
uniform mat4 og_viewportOrthographicMatrix;
uniform float og_perspectiveNearPlaneDistance;
uniform sampler2D og_heightMap;
uniform float u_fillDistance;
uniform float u_heightExaggeration;
uniform float u_positionToTextureCoordinate;
uniform int u_normalAlgorithm;

vec4 clipToWindowCoordinates( vec4 v ) {
	// normalized device coordinates
    v.xyz /= v.w;
    // window coordinates
    v.xyz = ( og_viewportTransformationMatrix * vec4( v.xyz, 1.0 ) ).xyz;
    return v;
}

void clipLineSegmentToNearPlane( vec4 modelP0, vec4 modelP1,
								 out vec4 clipP0, out vec4 clipP1,
								 out bool culledByNearPlane ) {
    clipP0 = og_modelViewPerspectiveMatrix * modelP0;
    clipP1 = og_modelViewPerspectiveMatrix * modelP1;
	culledByNearPlane = false;

    float distanceToP0 = clipP0.z + og_perspectiveNearPlaneDistance;
    float distanceToP1 = clipP1.z + og_perspectiveNearPlaneDistance;

    if( ( distanceToP0 * distanceToP1 ) < 0.0 ) {
        float t = distanceToP0 / ( distanceToP0 - distanceToP1 );
        vec3 modelV = vec3( modelP0 ) + t * ( vec3( modelP1 ) - vec3( modelP0 ) );
        vec4 clipV = og_modelViewPerspectiveMatrix * vec4( modelV, 1 );

        if( distanceToP0 < 0.0 ) {
            clipP0 = clipV;
        } else {
            clipP1 = clipV;
        }
    } else if( distanceToP0 < 0.0 )	{
	    culledByNearPlane = true;
	}
}

// Intake is displaced position
vec3 computeNormalForwardDifference( vec3 pos ) {
	float heightRight = texture( og_heightMap, 
		vec2( pos.x + 1.0, pos.z ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
    vec3 right = vec3( pos.x + 1.0, heightRight, pos.z );
    float heightTop = texture( og_heightMap, 
    	vec2( pos.x, pos.z + 1.0 ) * u_positionToTextureCoordinate ).r * u_heightExaggeration;
    vec3 top = vec3( pos.x, heightTop, pos.z + 1.0 );
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
    return -cross( right - left, top - bottom );
}

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
	
	float x = upperRight + ( 2.0 * right ) + lowerRight - upperLeft - ( 2.0 * left ) - lowerLeft;
    float y = lowerLeft + ( 2.0 * lowerCenter ) + lowerRight - upperLeft - ( 2.0 * upperCenter ) - upperRight;
    
    // These normals are correct side up ...
	return vec3( -x, 1.0, y );
}

void main() {
    vec3 terrainNormal = vec3(0.0);
    if( u_normalAlgorithm == 1 ) {
		// Forward difference
		terrainNormal = computeNormalForwardDifference( gl_in[0].gl_Position.xyz );
    } else if( u_normalAlgorithm == 2 ) {
		// Central Difference
		terrainNormal = computeNormalCentralDifference( gl_in[0].gl_Position.xyz );
    } else if( u_normalAlgorithm == 3 ) {
		// Sobel filter
        terrainNormal = computeNormalSobelFilter(gl_in[0].gl_Position.xyz );
    }

    vec4 clipP0;
    vec4 clipP1;
	bool culledByNearPlane;
    clipLineSegmentToNearPlane( gl_in[0].gl_Position, 
    							gl_in[0].gl_Position + vec4( normalize( terrainNormal ), 0.0 ),
								clipP0, clipP1, culledByNearPlane );
	if( culledByNearPlane ) {
		return;
	}

    vec4 windowP0 = clipToWindowCoordinates( clipP0 );
    vec4 windowP1 = clipToWindowCoordinates( clipP1 );

    vec2 direction = windowP1.xy - windowP0.xy;
    vec2 normal = normalize( vec2( direction.y, -direction.x ) );

    vec4 v0 = vec4( windowP0.xy - ( normal * u_fillDistance ), -windowP0.z, 1.0 );
    vec4 v1 = vec4( windowP1.xy - ( normal * u_fillDistance ), -windowP1.z, 1.0 );
    vec4 v2 = vec4( windowP0.xy + ( normal * u_fillDistance ), -windowP0.z, 1.0 );
    vec4 v3 = vec4( windowP1.xy + ( normal * u_fillDistance ), -windowP1.z, 1.0 );

    gl_Position = og_viewportOrthographicMatrix * v0;
    fsDistanceToEye = gsDistanceToEye[0];
    EmitVertex();

    gl_Position = og_viewportOrthographicMatrix * v1;
    fsDistanceToEye = gsDistanceToEye[0];
    EmitVertex();

    gl_Position = og_viewportOrthographicMatrix * v2;
    fsDistanceToEye = gsDistanceToEye[0];
    EmitVertex();

    gl_Position = og_viewportOrthographicMatrix * v3;
    fsDistanceToEye = gsDistanceToEye[0];
    EmitVertex();

}

