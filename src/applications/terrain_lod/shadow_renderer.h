
#pragma once

#include "cascaded_volume_map.h"
#include "quadtree.h"
#include "omath/mat4.h"
#include "glad/glad.h"
#include <cassert>

namespace terrain {

class cascaded_shadow_map : private cascaded_volume_map {
public:
	struct csm_layer : public cascaded_volume_map::layer {
		// This is the renderbuffer target for the shadow map		GLuint ShadowMap{0};
		GLuint ShadowMapDepth{0};
		omath::mat4 ShadowView;
		omath::mat4 ShadowProj;
		omath::mat4 ShadowViewProj;
		bool TaggedForRefresh{false};
		double TaggedForRefreshTime;
		float ApproxWorldTexelSize;
		float OrthoProjWidth;
		float OrthoProjHeight;
		float OrthoProjDepth;
		omath::vec3 OrthoEyePos;
		float WorldSpaceTexelSizeX;
		float WorldSpaceTexelSizeY;
		float NoiseScaleX;
		float NoiseScaleY;
		float SamplingTexelRadius;
		/*float LessDetailedLayerScaleX;
		float LessDetailedLayerScaleY;
		float NoiseDepthOffsetScale;*/
		virtual bool update( const omath::vec3 & observerPos, float newVisibilityRange,
				cascaded_volume_map* parent, bool forceUpdate = false );
		bool RenderShadowMap( void* renderContext, csm_layer * parentLayer );
	};

	cascaded_shadow_map();

	virtual ~cascaded_shadow_map();

	void setup();

	void render();

	/*bool RenderCascade( int cascadeIndex, DemoCamera * camera, int gridMeshDim,
			bool useDetailHeightmap, const CDLODQuadTree::LODSelection & terrainDLODSelection,
			CDLODRenderStats * renderStats );

	void Render( DemoCamera * camera, DemoSky * lightMgr, int gridMeshDim,
			const LODSelection& terrainDLODSelection, CDLODRenderStats * renderStats = NULL );*/

	void cleanup();

	//void UpdateShaderSettings( D3DXMACRO * newMacroDefines );

	const csm_layer& get_csm_layer( int index ) const {
		return m_cascades[index];
	}

private:
	//GLuint m_depthBuffer;
	GLuint m_scratchSurface;
	//@todo shader program
	GLuint m_vsShadow;
	GLuint m_psShadow;
	bool m_defPoolTexturesCreated{false};
	csm_layer m_cascades[NUMBER_OF_LOD_LEVELS];
	// @todo: check if this shouldn't be the z axis
	omath::vec3 m_lastShadowCamRightVec{ 0.0f, 1.0f, 0.0f };
	omath::vec3 m_shadowForward{ 0.0f, 0.0f, 0.0f };
	omath::vec3 m_shadowRight;
	omath::vec3 m_shadowUp;

};

}
