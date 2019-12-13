
#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec3 aPos;

out vec3 texCoords;

uniform mat4 projectionView;

void main() {
    texCoords = aPos;
    vec4 pos = projectionView * vec4( aPos, 1.0 );
	gl_Position = pos.xyww;
}
