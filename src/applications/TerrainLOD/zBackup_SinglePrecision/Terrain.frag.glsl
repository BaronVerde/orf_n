
#version 450 core

#pragma debug(on)

in VERTEX_OUTPUT {
	vec4 position;
	vec2 globalUV;
	vec3 lightDir;	// .xyz
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
	float lightFactor;
	vec3 normal;
	float morphLerpK;
} fragIn;

// actually diffuse and specular, but nevermind...
uniform vec4 g_lightColorDiffuse;
uniform vec4 g_lightColorAmbient;
uniform vec4 g_fogColor;
uniform vec4 g_colorMult;

uniform vec3 debugColor;

out vec4 fragColor;

float calculateDiffuseStrength( vec3 normal, vec3 lightDir ) {
	return clamp( -dot( normal, lightDir ), 0.0f, 1.0f );
}

float calculateSpecularStrength( vec3 normal, vec3 lightDir, vec3 eyeDir ) {
	float diff = clamp( dot( normal, -lightDir ), 0.0f, 1.0f );
	vec3 reflection = normalize( 2 * diff * normal + lightDir ); 
	return clamp( dot( reflection, eyeDir ), 0.0f, 1.0f );
}

float calculateDirectionalLight( vec3 normal, vec3 lightDir, vec3 eyeDir, 
		float specularPow, float specularMul ) {
	vec3 light0 = normalize( lightDir );
	return calculateDiffuseStrength( normal, light0 ) + specularMul * 
			pow( calculateSpecularStrength( normal, light0, eyeDir ), specularPow );
}

void terrainShader() {
	// normal.xz = normal.xz * vec2( 2.0, 2.0 ) - vec2( 1.0, 1.0 );
	// normal.y = sqrt( 1 - normal.x * normal.x - normal.z * normal.z );
	float directionalLight = calculateDirectionalLight( fragIn.normal, normalize( fragIn.lightDir ),
								normalize( fragIn.eyeDir.xyz ), 16.0f, 0.0f );
	vec4 color = vec4( g_lightColorAmbient.xyz + g_lightColorDiffuse.xyz * directionalLight, 1.0f );
	fragColor = color * g_colorMult;
}

void main() {
	terrainShader();
}
