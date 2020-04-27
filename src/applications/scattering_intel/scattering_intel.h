
#pragma once

#include "scene/Renderable.h"
#include "renderer/Framebuffer.h"

namespace scattering {

class scattering_intel : public orf_n::Renderable {
public:
	scattering_intel();

	virtual ~scattering_intel();

	virtual void setup() override final;

	virtual void prepareFrame() override final;

	virtual void render() override final;

	virtual void cleanup() override final;

private:
	void build_gui_window() const;
	bool CreateShadowMap();
	void ReleaseShadowMap();
	//void RenderShadowMap(ID3D11DeviceContext *pContext, SLightAttribs &LightAttribs);
	float GetSceneExtent();
	class CLightSctrPostProcess* m_pLightSctrPP{nullptr};
	unsigned int m_uiShadowMapResolution{1024};
	float m_fCascadePartitioningFactor{0.95f};
	bool m_bEnableLightScattering{true};
	bool m_bAnimateSun{false};
	//SPostProcessingAttribs m_PPAttribs;
	float m_fScatteringScale{0.5f};
	//std::vector<CComPtr<ID3D11DepthStencilView> > m_pShadowMapDSVs;
	//CComPtr<ID3D11ShaderResourceView> m_pShadowMapSRV;

	//@todo: These may be two different framebuffers !
	orf_n::Framebuffer* m_pOffscreenRenderTarget{nullptr};
	//CPUTRenderTargetColor* m_pOffscreenRenderTarget{NULL};
	//CPUTRenderTargetDepth* m_pOffscreenDepth{NULL};

	//CPUTCamera* m_pDirectionalLightCamera;
	//CPUTCamera* m_pDirLightOrienationCamera;
	//CPUTCameraController* mpCameraController;
	//CPUTCameraController* m_pLightController;
	//CPUTTimerWin m_Timer;
	float m_fElapsedTime;
	omath::vec4 m_f4LightColor{1.0f};
	int m_iGUIMode{1};
	//SRenderingParams m_TerrainRenderParams;
	std::string m_strRawDEMDataFile;
	std::string m_strMtrlMaskFile;
	//std::string m_strTileTexPaths[CEarthHemsiphere::NUM_TILE_TEXTURES];
	//std::string m_strNormalMapTexPaths[CEarthHemsiphere::NUM_TILE_TEXTURES];
	//std::auto_ptr<CElevationDataSource> m_pElevDataSource;
	//CEarthHemsiphere m_EarthHemisphere;
	omath::mat4 m_CameraViewMatrix;
	omath::vec3 m_CameraPos{0.0f};
	//CComPtr<ID3D11Buffer> m_pcbLightAttribs;
	unsigned int m_uiBackBufferWidth{0};
	unsigned int m_uiBackBufferHeight{0};
	//CPUTDropdown* m_pSelectPanelDropDowns[3];
	unsigned int m_uiSelectedPanelInd{0};
	float m_fMinElevation;
	float m_fMaxElevation;
	//COutdoorLightScatteringSample(const COutdoorLightScatteringSample&);
	//const COutdoorLightScatteringSample& operator = (const COutdoorLightScatteringSample&);

};

}
