
#version 450 core

#pragma debug(on)

uniform vec4 debugColor;

out vec4 fragColor;

void main() {
	fragColor = debugColor;
}
