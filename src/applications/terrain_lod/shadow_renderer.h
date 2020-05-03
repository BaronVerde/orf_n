
#pragma once

#include <applications/terrain_lod/cascaded_volume_map.h>
#include <applications/terrain_lod/quadtree.h>
#include "omath/mat4.h"
#include <cassert>

namespace terrain {

class cascaded_shadow_map : private cascaded_volume_map {
public:
	struct csm_layer : public cascaded_volume_map::layer {
		//IDirect3DTexture9 * ShadowMap;
		//IDirect3DTexture9 * ShadowMapDepth;
		omath::mat4 ShadowView;
		omath::mat4 ShadowProj;
		omath::mat4 ShadowViewProj;
		bool TaggedForRefresh;
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
		csm_layer() {
			//ShadowMap = NULL;
			//ShadowMapDepth = NULL;
			TaggedForRefresh = false;
		}
		virtual ~csm_layer() {
			//SAFE_RELEASE( ShadowMap );
			//SAFE_RELEASE( ShadowMapDepth );
		}
		virtual bool update( const omath::vec3 & observerPos, float newVisibilityRange,
				cascaded_volume_map* parent, bool forceUpdate = false );

		bool RenderShadowMap( void* renderContext, csm_layer * parentLayer );
	};

	cascaded_shadow_map();

	virtual ~cascaded_shadow_map();

	void setup();

	void InitializeRuntimeData( );
	bool RenderCascade( int cascadeIndex, DemoCamera * camera, int gridMeshDim,
			bool useDetailHeightmap, const CDLODQuadTree::LODSelection & terrainDLODSelection,
			CDLODRenderStats * renderStats );

	void Deinitialize();

	void Render( DemoCamera * camera, DemoSky * lightMgr, int gridMeshDim,
			const LODSelection& terrainDLODSelection/*, CDLODRenderStats * renderStats = NULL*/ );

	void UpdateShaderSettings( D3DXMACRO * newMacroDefines );

	const csm_layer & GetCascadeLayer( int index ) const {
		assert( index >= 0 && index < m_layer_count );
		return m_cascades[index];
	}
	const csm_layer & GetCascadeLayerForDLODLevel( int dlodLevel ) const {
		return GetCascadeLayer( dlodLevel );
	}
	int GetCascadeLayerCount( ) const {
		return m_layer_count;
	}
	int GetTextureResolution() const {
		return m_textureResolution;
	}

private:
	//IDirect3DSurface9 * m_depthBuffer;
	//IDirect3DSurface9 * m_scratchSurface;
	//const DemoRenderer * m_mainRenderer;
	//DxVertexShader m_vsShadow;
	//DxPixelShader m_psShadow;
	bool m_defPoolTexturesCreated;
	//int m_DLODLevelsPerCascade;
	csm_layer m_cascades[NUMBER_OF_LOD_LEVELS];
	omath::vec3 m_lastShadowCamRightVec;
	omath::vec3 m_shadowForward;
	omath::vec3 m_shadowRight;
	omath::vec3 m_shadowUp;
	int m_textureResolution;

};

}
