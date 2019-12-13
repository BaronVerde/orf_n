
#version 450 core

#pragma debug(on)

const float c_pi = 3.14159265f;
const float c_oneOverPi = 1.0f / c_pi;
const float c_oneOverTwoPi = 0.5f / c_pi;

layout( location = 0 ) in vec3 inPositionHigh;
layout( location = 1 ) in vec3 inPositionLow;

layout( binding = 0 ) uniform sampler2D s_texture;

layout( location = 5 ) uniform vec3 u_cameraPositionHigh;
layout( location = 6 ) uniform vec3 u_cameraPositionLow;
layout( location = 18 ) uniform mat4 u_mvpMatrixRTE;

uniform bool u_showDisplacement;
uniform vec3 u_oneOverRadiiSquared;
uniform vec4 u_drawColor;

out vec3 worldPosition;
out vec3 drawColor;

vec3 geodeticSurfaceNormal( const vec3 pos ) {
	return normalize( pos * u_oneOverRadiiSquared );
}

vec2 computeTexCoord( const vec3 normal ) {
	return vec2( atan( normal.y, normal.x ) * c_oneOverTwoPi + 0.5f, asin( normal.z ) * c_oneOverPi + 0.5f );
}

/* Calculate position rte from world position parts high/low and deduct camera position high/low.
 * Moves the world position into camera space. Result is single precision float */
vec3 calculatePositionRTE() {	
	vec3 t1 = inPositionLow - u_cameraPositionLow;
	vec3 e = t1 - inPositionLow;
	vec3 t2 = ( ( -u_cameraPositionLow - e ) + ( inPositionLow - ( t1 - e ) ) ) + inPositionHigh - u_cameraPositionHigh;
	vec3 highDifference = t1 + t2;
	vec3 lowDifference = t2 - ( highDifference - t1 );
	return highDifference + lowDifference;
}

/* For addition instead of subtraction change these signs:
 * https://www.thasler.com/blog/blog/glsl-part2-emu
	vec3 t1 = inPositionLow + u_cameraPositionLow;
	vec3 e = t1 - inPositionLow;
	vec3 t2 = ( ( u_cameraPositionLow - e ) + ( inPositionLow - ( t1 - e ) ) ) + inPositionHigh + u_cameraPositionHigh;
	vec3 highDifference = t1 + t2;
	vec3 lowDifference = t2 - ( highDifference - t1 );
	return highDifference + lowDifference; */
	
/* Multiplication is weird (same source):
*/

void main() {
	vec3 positionRTE = calculatePositionRTE();

	//vec3 normal = geodeticSurfaceNormal( position );		
	//vec2 texCoord = computeTexCoord( normal );
	
	drawColor = u_drawColor.xyz;
	gl_Position = u_mvpMatrixRTE * vec4( positionRTE, 1.0f );
	//gl_Position = u_perspectiveMatrix * u_mvMatrixRTE * vec4( positionRTE, 1.0f );
}
