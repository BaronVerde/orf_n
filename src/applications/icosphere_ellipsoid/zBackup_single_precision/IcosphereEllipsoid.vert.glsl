
#version 450 core

#pragma debug(on)

const float c_pi = 3.14159265f;
const float c_oneOverPi = 1.0f / c_pi;
const float c_oneOverTwoPi = 0.5f / c_pi;

layout( location = 0 ) in vec3 position;

layout( binding = 0 ) uniform sampler2D s_texture;

layout( location = 5 ) uniform vec3 u_cameraPosition;
layout( location = 7 ) uniform mat4 u_modelMatrix;
layout( location = 15 ) uniform mat4 u_viewProjectionMatrix;

uniform bool u_showDisplacement;
uniform vec3 u_oneOverRadiiSquared;
uniform int u_normalType;

out vec3 worldPosition;
out vec3 drawColor;

vec3 centricSurfaceNormal( const vec3 pos ) {
	return normalize( pos );
}

vec3 geodeticSurfaceNormal( const vec3 pos ) {
	return normalize( pos * u_oneOverRadiiSquared );
}

vec2 computeTexCoord( const vec3 normal ) {
	return vec2( atan( normal.y, normal.x ) * c_oneOverTwoPi + 0.5f, asin( normal.z ) * c_oneOverPi + 0.5f );
}

void main() {
	vec3 displacedPosition;
	vec3 normal;
	if( u_showDisplacement ) {
		if( 0 == u_normalType )
			normal = centricSurfaceNormal( position );
		else if( 1 == u_normalType )
			normal = geodeticSurfaceNormal( position );
		else
			normal = ( geodeticSurfaceNormal( position ) + centricSurfaceNormal( position ) ) / 2.0f;		
		vec2 texCoord = computeTexCoord( normal );
		float height = texture( s_texture, texCoord ).r / 30.0;
		displacedPosition = position + ( normal * height );
		drawColor = vec3( 1.0f, 0.0f, 0.0f );
	} else {
		displacedPosition = position;
		drawColor = vec3( 0.0f, 1.0f, 0.0f );
	}
	gl_Position = u_viewProjectionMatrix * vec4( displacedPosition, 1.0 );
	worldPosition = displacedPosition;
}
