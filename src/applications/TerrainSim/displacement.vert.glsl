
#version 450 core

layout( location = 0 ) in vec3 position;

layout( location = 5 ) uniform vec3 u_cameraPosition;
layout( location = 10 ) uniform mat4 u_modelViewPerspectiveMatrix;
//layout( location = 11 ) uniform vec3 u_sunPosition;

layout( binding = 1 ) uniform sampler2D s_liftrateMap;
uniform vec2 u_posToLiftrateMap;

uniform float u_liftrate;
uniform float u_timeStep = 1;

void main() {
	float offset = texture( s_liftrateMap, vec2( position.x, position.z ) ).r;
	offset *= u_timeStep * u_liftrate;

	gl_Position = u_modelViewPerspectiveMatrix * vec4( position.x, offset, position.z, 1.0 );

}

