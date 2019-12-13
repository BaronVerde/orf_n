
#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec3 inPosition;

layout( binding = 1 ) uniform sampler2D positionsHigh;
layout( binding = 2 ) uniform sampler2D positionsLow;

layout( location = 5 ) uniform vec3 u_cameraPositionHigh;
layout( location = 6 ) uniform vec3 u_cameraPositionLow;
layout( location = 15 ) uniform mat4 u_viewProjectionMatrix;
layout( location = 18 ) uniform mat4 u_mvpMatrixRTE;

uniform vec4 u_drawColor;

out vec3 worldPosition;
out vec3 drawColor;

/* Calculate position rte from world position parts high/low and deduct camera position high/low.
 * Moves the world position into camera space. Result is single precision float */
vec3 calculatePositionRTE( vec3 posHigh, vec3 posLow ) {
	vec3 t1 = posLow - u_cameraPositionLow;
	vec3 e = t1 - posLow;
	vec3 t2 = ( ( -u_cameraPositionLow - e ) + ( posLow - ( t1 - e ) ) ) + posHigh - u_cameraPositionHigh;
	vec3 highDifference = t1 + t2;
	vec3 lowDifference = t2 - ( highDifference - t1 );
	return highDifference + lowDifference;
}

void main() {
	drawColor = u_drawColor.xyz;	
	vec3 posHigh = texture( positionsHigh, inPosition.xz ).rgb;
	vec3 posLow = texture( positionsLow, inPosition.xz ).rgb;
	vec3 positionRTE = calculatePositionRTE( posHigh, posLow );
	gl_Position = u_mvpMatrixRTE * vec4( positionRTE, 1.0f );
	//gl_Position = u_viewProjectionMatrix * vec4( inPosition, 1.0f );
}
