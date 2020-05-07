
#include <applications/terrain_lod/sky.h>
#include "base/globals.h"

extern double orf_n::globals::delta_time;

namespace terrain {

sky_renderer::sky_renderer() {
	// initial pos
	m_directionalLightDir = omath::vec3{0.0f};
	m_sunAzimuth = 200.0f / 180.0f * (float)M_PI;
	m_sunElevation = 30.0f / 180.0f * (float)M_PI;
	m_directionalLightDirTarget = omath::vec3{0.0f};
	m_directionalLightDirTargetL1 = omath::vec3{0.0f};
	// do an icosphere
	m_sun_mesh = new orf_n::icosphere( omath::vec3{300.0f}, 5 );
	glCreateVertexArrays( 1, &m_vertex_array );
	glCreateBuffers( 1, &m_vertex_buffer );
	glCreateBuffers( 1, &m_index_buffer );
	glVertexArrayVertexBuffer(
			m_vertex_array, VERTEX_BUFFER_BINDING_INDEX, m_vertex_buffer, 0, sizeof(omath::vec3)
	);
	glNamedBufferData(
			m_vertex_buffer, m_sun_mesh->get_vertices.size() * sizeof(GLuint),
			m_sun_mesh->get_vertices().data(), GL_STATIC_DRAW
	);
	glVertexArrayAttribBinding( m_vertex_array, 0, VERTEX_BUFFER_BINDING_INDEX );
	glVertexArrayAttribFormat( m_vertex_array, 0, 3, GL_FLOAT, GL_FALSE, 0 );
	glEnableVertexArrayAttrib( m_vertex_array, 0 );
	glNamedBufferData(
			m_index_buffer, m_sun_mesh->get_indices.size() * sizeof(GLuint),
			m_sun_mesh->get_indices().data(), GL_STATIC_DRAW
	);
	glVertexArrayElementBuffer( m_vertex_array, m_index_buffer );
}

sky_renderer::~sky_renderer() {
	glDisableVertexAttribArray( m_vertex_array, 0 );
	glDeleteBuffers( 1, &m_vertex_buffer );
	glDeleteBuffers( 1, &m_index_buffer );
	glDeleteVertexArrays( 1, &m_vertex_array );
	delete m_sun_mesh;
}

void sky_renderer::Tick() {
	// this smoothing is not needed here, but I'll leave it in anyway
	static float some_value = 1000.0f;
	float lf = omath::time_independent_lerp( (float)delta_time, some_value );
	if( omath::magnitude_sq( m_directionalLightDir ) < 1e-5f )
		lf = 1.0f;
	omath::mat4 mCameraRot;
	omath::mat4 mRotationY; D3DXMatrix4RotationY( &mRotationY, m_sunElevation );
	omath::mat4 mRotationZ; D3DXMatrixRotationZ( &mRotationZ, m_sunAzimuth );
	mCameraRot = mRotationY * mRotationZ;
	m_directionalLightDirTarget = omath::vec3( mCameraRot.m[0] );
	D3DXVec3Lerp( &m_directionalLightDirTargetL1, &m_directionalLightDirTargetL1, &m_directionalLightDirTarget, lf );
	D3DXVec3Lerp( &m_directionalLightDir, &m_directionalLightDir, &m_directionalLightDirTargetL1, lf );
	D3DXVec3Normalize( &m_directionalLightDirTarget, &m_directionalLightDirTarget );
	D3DXVec3Normalize( &m_directionalLightDirTargetL1, &m_directionalLightDirTargetL1 );
	D3DXVec3Normalize( &m_directionalLightDir, &m_directionalLightDir );
}

/*void sky_renderer::Render( DemoCamera * pCamera ) {
	omath::mat4 view = pCamera->GetViewMatrix();
	omath::mat4 proj = pCamera->GetProjMatrix();
	view._41 = 0.0; view._42 = 0.0; view._43 = 0.0;
	//omath::mat4 viewProj = view * proj;
	D3DXMATRIXA16 matWorld;
	D3DXMatrixTranslation(
			&matWorld, m_directionalLightDir.x * -1e4f, m_directionalLightDir.y * -1e4f, m_directionalLightDir.z * -1e4f
	);
	pDevice->SetTransform( D3DTS_WORLD, &matWorld );
	pDevice->SetTransform( D3DTS_VIEW, &view );
	pDevice->SetTransform( D3DTS_PROJECTION, &proj );
	D3DMATERIAL9 sunMat;
	sunMat.Ambient.r = 1.0f; sunMat.Ambient.g = 1.0f; sunMat.Ambient.b = 1.0f; sunMat.Ambient.a = 1.0f;
	sunMat.Diffuse.r = 1.0f; sunMat.Diffuse.g = 1.0f; sunMat.Diffuse.b = 1.0f; sunMat.Diffuse.a = 1.0f;
	sunMat.Emissive.r = 1.0f; sunMat.Emissive.g = 1.0f; sunMat.Emissive.b = 1.0f; sunMat.Emissive.a = 1.0f;
	sunMat.Specular.r = 1.0f; sunMat.Specular.g = 1.0f; sunMat.Specular.b = 1.0f; sunMat.Specular.a = 1.0f;
	sunMat.Power = 1.0f;
	pDevice->SetFVF( m_sunMesh->GetFVF() );
	pDevice->SetMaterial( &sunMat );
	pDevice->SetTexture( 0, NULL );
	pDevice->SetVertexShader( NULL );
	pDevice->SetPixelShader( NULL );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	m_sunMesh->DrawSubset( 0 );
}*/

}
