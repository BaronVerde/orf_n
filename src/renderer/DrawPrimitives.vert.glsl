
#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec3 position;

uniform mat4 modelMatrix;
uniform mat4 projViewMatrix;
uniform mat4 mvpMatrixRTE;

uniform bool useRTE = false;

void main() {
	gl_Position = projViewMatrix * modelMatrix * vec4( position, 1.0f );
}
