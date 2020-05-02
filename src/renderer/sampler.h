
#pragma once

#include "glad/glad.h"

/**
 * Profrabricated samplers for textures.
 * Sets the sampler parameters for the texture given as argument.
 */

namespace orf_n {

typedef enum {
	NEAREST_CLAMP,
	LINEAR_CLAMP,
	NEAREST_REPEAT,
	LINEAR_REPEAT
} sampler_type_t;

static inline void set_default_sampler( GLuint texture, sampler_type_t type ) {
	switch( type ) {
		case NEAREST_CLAMP:
			glTextureParameteri( texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTextureParameteri( texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			break;
		case LINEAR_CLAMP:
			glTextureParameteri( texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTextureParameteri( texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			break;
		case NEAREST_REPEAT:
			glTextureParameteri( texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTextureParameteri( texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_T, GL_REPEAT );
			break;
		case LINEAR_REPEAT:
			glTextureParameteri( texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTextureParameteri( texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTextureParameteri( texture, GL_TEXTURE_WRAP_T, GL_REPEAT );
			break;
	}
}

}
