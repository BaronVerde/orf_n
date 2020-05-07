 
#version 450 core

#pragma debug(on)

layout( location = 11 ) uniform mat4 g_projectionMatrix;

layout( location = 0) in vec2 Position;
layout( location = 1) in vec2 UV;
layout( location = 2) in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

void main() {
	Frag_UV = UV;
	Frag_Color = Color;
	gl_Position = g_projectionMatrix * vec4(Position.xy,0,1);
}
