
#version 450 core

#pragma debug(on)

in vec3 worldPosition;
in vec3 drawColor;

out vec4 fragmentColor;

void main() {
	fragmentColor = vec4( drawColor, 1.0f );
}
