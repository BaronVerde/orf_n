
﻿#version 450 core

#pragma debug(on)
                 
in float fsDistanceToEye;

out vec4 fragmentColor;

void main() {
	// Apply linear attenuation to alpha
    float a = min( 1.0 / ( 0.015 * fsDistanceToEye ), 1.0 );
	if( a == 0.0 ) {
		discard;
	}

	// red
    fragmentColor = vec4( 1.0, 0.0, 0.0, a );

}

