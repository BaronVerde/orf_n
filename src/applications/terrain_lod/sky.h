
#pragma once

#include "omath/vec3.h"
#include "geometry/icosphere.h"

namespace terrain {

// Manages directional light, drawing of the sun, etc
class sky_renderer {
private:

	const GLuint VERTEX_BUFFER_BINDING_INDEX{0};

	omath::vec3 m_directionalLightDir;
	omath::vec3 m_directionalLightDirTarget;
	omath::vec3 m_directionalLightDirTargetL1;
	// @todo do an icosphere
	orf_n::icosphere* m_sun_mesh;
	GLuint m_vertex_array;
	GLuint m_vertex_buffer;
	GLuint m_index_buffer;
	float m_sunAzimuth;
	float m_sunElevation;

public:
	sky_renderer();

	virtual ~sky_renderer();

	const omath::vec3& GetDirectionalLightDir() {
		return m_directionalLightDir;
	}

	void Tick( float deltaTime );

	//void Render( DemoCamera * pCamera );

	void GetSunPosition( float& azimuth, float& elevation ) const {
		azimuth = m_sunAzimuth;
		elevation = m_sunElevation;
	}

	void SetSunPosition( const float azimuth, const float elevation ) {
		m_sunAzimuth = azimuth;
		m_sunElevation = elevation;
	}

};

}
