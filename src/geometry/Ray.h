
#pragma once

#include <geometry/OBB.h>
#include <omath/vec3.h>

namespace orf_n {

typedef struct {
	omath::vec3 point{ 0.0f, 0.0f, 0.0f };
	omath::vec3 normal{ 0.0f, 0.0f, 1.0f };
	float t{ -1.0f };
	bool hit{ false };

	void reset() {
		point = omath::vec3{ 0.0f, 0.0f, 0.0f };
		normal = omath::vec3{ 0.0f, 0.0f, 1.0f };
		t = -1.0f;
		hit = false;
	}

} raycastResult_t;

class Ray {
public:
	Ray();

	Ray( const omath::vec3 &origin, const omath::vec3 &direction );

	void normalizeDirection();

	bool raycast( const OBB &obb, raycastResult_t *outResult );

private:
	omath::vec3 m_origin{ 0.0f, 0.0f, 0.0f };

	omath::vec3 m_direction{ 0.0f, 0.0f, 1.0f };

};

}
