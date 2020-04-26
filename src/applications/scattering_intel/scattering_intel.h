
#pragma once

#include "scene/Renderable.h"

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
	bool CreateShadowMap();
	void ReleaseShadowMap();
	bool CreateTmpBackBuffAndDepthBuff();
	void ReleaseTmpBackBuffAndDepthBuff();
	//void RenderShadowMap(ID3D11DeviceContext *pContext, SLightAttribs &LightAttribs);
	float GetSceneExtent();
	class CLightSctrPostProcess *m_pLightSctrPP;
	unsigned int m_uiShadowMapResolution;
	float m_fCascadePartitioningFactor;
	bool m_bEnableLightScattering;
	bool m_bAnimateSun;
	static const int m_iMinEpipolarSlices = 32;
	static const int m_iMaxEpipolarSlices = 2048;
	static const int m_iMinSamplesInEpipolarSlice = 32;
	static const int m_iMaxSamplesInEpipolarSlice = 2048;
	static const int m_iMaxEpipoleSamplingDensityFactor = 32;
	static const int m_iMinInitialSamplesInEpipolarSlice = 8;
	//SPostProcessingAttribs m_PPAttribs;
	float m_fScatteringScale;
	//std::vector<CComPtr<ID3D11DepthStencilView> > m_pShadowMapDSVs;
	//CComPtr<ID3D11ShaderResourceView> m_pShadowMapSRV;
	//CPUTRenderTargetColor*  m_pOffscreenRenderTarget;
	//CPUTRenderTargetDepth*  m_pOffscreenDepth;
	//CPUTCamera*           m_pDirectionalLightCamera;
	//CPUTCamera*           m_pDirLightOrienationCamera;
	//CPUTCameraController* mpCameraController;
	//CPUTCameraController* m_pLightController;
	//CPUTTimerWin          m_Timer;
	float                 m_fElapsedTime;
	omath::vec4 m_f4LightColor;
	int m_iGUIMode;
	//SRenderingParams m_TerrainRenderParams;
	std::string m_strRawDEMDataFile;
	std::string m_strMtrlMaskFile;
	//std::string m_strTileTexPaths[CEarthHemsiphere::NUM_TILE_TEXTURES];
	//std::string m_strNormalMapTexPaths[CEarthHemsiphere::NUM_TILE_TEXTURES];
	//std::auto_ptr<CElevationDataSource> m_pElevDataSource;
	//CEarthHemsiphere m_EarthHemisphere;
	omath::mat4 m_CameraViewMatrix;
	omath::vec3 m_CameraPos;
	//CComPtr<ID3D11Buffer> m_pcbLightAttribs;
	unsigned int m_uiBackBufferWidth, m_uiBackBufferHeight;
	//CPUTDropdown* m_pSelectPanelDropDowns[3];
	unsigned int m_uiSelectedPanelInd;
	float m_fMinElevation, m_fMaxElevation;
	//COutdoorLightScatteringSample(const COutdoorLightScatteringSample&);
	//const COutdoorLightScatteringSample& operator = (const COutdoorLightScatteringSample&);

};

}
