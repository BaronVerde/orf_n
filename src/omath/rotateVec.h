
#pragma once

#include <omath/mat4.h>

namespace omath {

template <typename T>
static inline vec3_t<T> rotate ( vec3_t<T> const& v, T const& angle, vec3_t<T> const& normal ) {
	mat3_t<T> rm{ rotate( mat4_t<T>{ 1.0f }, angle, normal ) };
	return rm * v;
}

}
