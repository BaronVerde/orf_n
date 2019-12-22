
#version 450 core

layout( location = 0 ) in vec2 flatPosition;

layout( location = 5 ) uniform vec3 og_cameraPosition;
layout( location = 9 ) uniform mat4 og_modelViewPerspectiveMatrix;
layout( location = 11 ) uniform vec3 og_sunPosition;

layout( binding = 1 ) uniform sampler2D og_heightMap;

uniform float u_heightExaggeration;

