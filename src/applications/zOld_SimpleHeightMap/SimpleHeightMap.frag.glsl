
#version 450 core

#pragma debug(on)

in VS_DATA {
	vec3 normal;
	vec3 positionToLight;
	vec3 positionToEye;
	vec2 textureCoordinate;
	vec2 repeatTextureCoordinate;
	float height;
} fs_in;

layout( location = 12 ) uniform vec3 og_sunColor;
layout( location = 13 ) uniform vec4 og_diffSpecAmbShininess;

// Min and max height scaled to [0..1]
uniform float u_minHeight;
uniform float u_minHeightClamped;
uniform float u_maxHeight;
uniform float u_maxHeightClamped;
uniform int u_shadingAlgorithm;
uniform int u_normalAlgorithm;
uniform float og_highResolutionSnapScale;
uniform bool u_showTerrain;
uniform bool u_showSilhouette;

layout( binding = 6 ) uniform sampler2D og_colorMap;
layout( binding = 1 ) uniform sampler2D og_colorRamp;
layout( binding = 7 ) uniform sampler2D og_slopeRamp;   // Color ramp for slope
layout( binding = 2 ) uniform sampler2D og_blendRamp;   // Blend ramp for grass and stone
layout( binding = 3 ) uniform sampler2D og_grass;    	// Grass
layout( binding = 4 ) uniform sampler2D og_stone;		// Stone
layout( binding = 5 ) uniform sampler2D og_blendMap;    // Blend map

layout( location = 0 ) out vec4 fragmentColor;

// Standard diff/spec/amb term
float lightIntensity( vec3 normal, vec3 toLight, vec3 toEye ) {
    vec3 toReflectedLight = reflect( -toLight, normal );
    float diffuse = max( dot( toLight, normal ), 0.0 );
    float specular = max( dot( toReflectedLight, toEye ), 0.0 );
    specular = pow( specular, og_diffSpecAmbShininess.w );
    return( og_diffSpecAmbShininess.x * diffuse +
			og_diffSpecAmbShininess.y * specular +
			og_diffSpecAmbShininess.z );
}

void main() {
	vec3 normal = normalize( fs_in.normal );
    vec3 positionToLight = normalize( fs_in.positionToLight );
    vec3 positionToEye = normalize( fs_in.positionToEye );
	//float absHeight = fs_in.height * 255.0;
	float absHeight = fs_in.height * 65535.0;
    
    if( u_showSilhouette ) {
        if( abs( dot( normal, positionToEye ) ) < 0.25 ) {
            fragmentColor = vec4( 0.0 );
            return;
        }
    }

    if( !u_showTerrain ) {
        discard;
    }
    
    // Full intensity if no normal algorithm is given
	float intensity = 1.0;
	vec3 fColor = vec3( intensity );
    // Light intensity if normal algorithm is given
    if( 0 != u_normalAlgorithm ) {
        intensity = lightIntensity( normal, positionToLight, positionToEye );
    }
    if( 0 == u_shadingAlgorithm ) {
        // terrain shading color map
        fColor = intensity * texture( og_colorMap, fs_in.textureCoordinate ).rgb;
    } else if( 1 == u_shadingAlgorithm ) {
    	// solid
		fColor = vec3( 0.0, intensity, 0.0 );
    } else if( 2 == u_shadingAlgorithm ) {
	    // By height. @todo: stretch from min height to max height
		fColor = vec3( 0.5, 
					   intensity * ( ( absHeight - u_minHeight ) / ( u_maxHeight - u_minHeight ) ),
					   0.5 );
	} else if( 3 == u_shadingAlgorithm ) {
		// By height contour. Contour every 10 meters @todo: stretch from min height to max height
        float distanceToContour = mod( absHeight, 10.0 );
        float dx = abs( dFdx( absHeight ) );
        float dy = abs( dFdy( absHeight ) );
        // line width
        float dF = max( dx, dy ) * og_highResolutionSnapScale * 2.0;
        fColor = mix( vec3( 0.0, intensity, 0.0 ), vec3( intensity, 0.0, 0.0 ), distanceToContour < dF );
	} else if( 4 == u_shadingAlgorithm ) {
		// Color ramp by height
		fColor = intensity * texture( og_colorRamp, vec2( 0.5, ( absHeight - u_minHeight ) / 
															   ( u_maxHeight - u_minHeight ) ) ).rgb;
    } else if ( 5 == u_shadingAlgorithm ) {
		// Blend ramp by height
		float normalizedHeight = ( absHeight - u_minHeight ) / ( u_maxHeight - u_minHeight );
		// float normalizedHeight = fs_in.height;
		fColor = intensity * mix( texture( og_grass, fs_in.repeatTextureCoordinate ).rgb,
								  texture( og_stone, fs_in.repeatTextureCoordinate ).rgb,
								  texture( og_blendRamp, vec2( 0.5, normalizedHeight ) ).r );
    } else if( 6 == u_shadingAlgorithm ) {
		// By slope
        fColor = vec3( fs_in.normal.z );
    } else if( 7 == u_shadingAlgorithm ) {
		// Slope contour
        float slopeAngle = acos( fs_in.normal.z );
        // Contour every 15 degrees
        float distanceToContour = mod( slopeAngle, radians( 15.0 ) );
        float dx = abs( dFdx( slopeAngle ) );
        float dy = abs( dFdy( slopeAngle ) );
        // Line width
        float dF = max( dx, dy ) * og_highResolutionSnapScale * 2.0;
		fColor = mix( vec3( 0.0, intensity, 0.0 ), vec3( intensity, 0.0, 0.0 ), ( distanceToContour < dF ) );
    } else if( 8 == u_shadingAlgorithm ) {
		// Color ramp by slope
        fColor = intensity * texture( og_slopeRamp, vec2( 0.5, fs_in.normal.z ) ).rgb;
    } else if( 9 == u_shadingAlgorithm ) {
		// Blend ramp by slope
        fColor = intensity * mix( texture( og_stone, fs_in.repeatTextureCoordinate ).rgb,	// Stone
								  texture( og_grass, fs_in.repeatTextureCoordinate ).rgb,	// Grass
								  texture( og_blendRamp, vec2( 0.5, fs_in.normal.z ) ).r );	// Blend mask
    } else if( 10 == u_shadingAlgorithm ) {
		// Blend mask
        fColor = intensity * mix( texture( og_grass, fs_in.repeatTextureCoordinate).rgb,	// Grass
								  texture( og_stone, fs_in.repeatTextureCoordinate).rgb,	// Stone
								  texture( og_blendMap, fs_in.textureCoordinate ).r );		// Blend mask
    }
    	
    fragmentColor = vec4( fColor, 1.0 );

}

