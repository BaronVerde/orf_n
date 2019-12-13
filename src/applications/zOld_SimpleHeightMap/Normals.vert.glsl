
﻿#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec2 flatPosition;

layout( binding = 0 ) uniform sampler2D og_heightMap;
                  
out float gsDistanceToEye;

layout( location = 5 ) uniform vec3 og_cameraPosition;
uniform float u_heightExaggeration;
uniform float u_positionToTextureCoordinate;

void main() {
	float height = texture( og_heightMap, flatPosition * u_positionToTextureCoordinate ).r;
    vec3 displacedPosition = vec3( flatPosition.x, height * u_heightExaggeration, flatPosition.y );
    gl_Position = vec4( displacedPosition, 1.0 );
    gsDistanceToEye = distance( displacedPosition, og_cameraPosition );
}

