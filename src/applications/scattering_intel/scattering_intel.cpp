
#include "scattering_intel.h"
#include "scattering_config.h"
#include "scene/Scene.h"
#include "applications/Camera/Camera.h"
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "base/Globals.h"

namespace scattering {

void UpdateConstantBuffer( GLuint buffer, const void *pData, size_t DataSize );

scattering_intel::scattering_intel() : Renderable{ "Scattering" } {
	//m_pLightSctrPP = new CLightSctrPostProcess;
}

scattering_intel::~scattering_intel() {
	cleanup();
	//delete m_pLightSctrPP;
}

void scattering_intel::setup() {
	// Initialize GUI -> prepareFrame or render method
	if( orf_n::globals::showAppUI )
		build_gui_window();

	int width = m_scene->getWindow()->getWidth();
	int height = m_scene->getWindow()->getHeight();
	// create framebuffer DXGI_FORMAT_R11G11B10_FLOAT and DXGI_FORMAT_D32_FLOAT
	m_pOffscreenRenderTarget = new orf_n::Framebuffer( width, height );
	m_pOffscreenRenderTarget->addColorAttachment( GL_R11F_G11F_B10F );
	m_pOffscreenRenderTarget->addDepthAttachment( GL_DEPTH_COMPONENT32F );
	if( !m_pOffscreenRenderTarget->isComplete() )
		std::cerr << "Framebuffer object incomplete" << std::endl;
	// Create shadow map before other assets!!!
	if( !CreateShadowMap() )
		std::cerr << "Error creating shadow map" << std::endl;

	// pAssetLibrary->SetMediaDirectoryName(  _L("Media\\"));
	// Add our programatic (and global) material parameters
	// CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerFrameValues"), _L("$cbPerFrameValues") );
	// CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerModelValues"), _L("#cbPerModelValues") );
	/*CPUTOSServices::GetOSServices()->GetClientDimensions(&width, &height);
	CPUTRenderStateBlockDX11 *pBlock = new CPUTRenderStateBlockDX11();
	CPUTRenderStateDX11 *pStates = pBlock->GetState();
	// Override default sampler desc for our default shadowing sampler
	pStates->SamplerDesc[1].Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pStates->SamplerDesc[1].AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
	pStates->SamplerDesc[1].AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
	pStates->SamplerDesc[1].ComparisonFunc = D3D11_COMPARISON_GREATER;
	pBlock->CreateNativeResources();
	CPUTAssetLibrary::GetAssetLibrary()->AddRenderStateBlock( _L("$DefaultRenderStates"), pBlock );
	pBlock->Release(); // We're done with it.  The library owns it now.*/
	// Initialize view camera
	m_scene->getCamera()->setZoom( 45.0f );
	m_scene->getCamera()->setNearPlane( 50.0f );
	m_scene->getCamera()->setFarPlane( 1e7f );
	m_scene->getCamera()->setPositionAndTarget( {0.0,8024,0.0 }, {0,0,0} );
	m_scene->getCamera()->setMovementSpeed( 200.0f );
	m_scene->getCamera()->updateMoving();

	// Create shadow camera
	/*m_pDirectionalLightCamera = new CParallelLightCamera();
	m_pDirLightOrienationCamera = new CParallelLightCamera();
	float4x4 LightOrientationWorld (
			-0.92137718f, -0.36748588f,  -0.12656364f, 0.0000000f,
			-0.37707147f,  0.92411846f,  0.061823435f, 0.0000000f,
			0.094240554f,  0.10468624f,  -0.99003011f, 0.0000000f,
			0.f,          0.f,           0.f, 1.0000000f
	);
	m_pDirLightOrienationCamera->SetParentMatrix( LightOrientationWorld );
	m_pDirLightOrienationCamera->Update();
	m_pLightController = new CPUTCameraControllerArcBall();
	m_pLightController->SetCamera( m_pDirLightOrienationCamera );
	m_pLightController->SetLookSpeed(0.002f);*/
	// Creates some resources that our blur material needs (e.g., the back buffer)

	m_uiBackBufferWidth = width;
	m_uiBackBufferHeight = height;
	m_pLightSctrPP->OnResizedSwapChain( width, height );
	//m_pOffscreenRenderTarget->RecreateRenderTarget( width, height );
	//m_pOffscreenDepth->RecreateRenderTarget( width, height );

	// Initialize the post process object
	if( !m_pLightSctrPP->OnCreateDevice() )
		std::cerr << "Error initializing post process object" << std::endl;
	// Create data source
	m_pElevDataSource.reset( new CElevationDataSource(m_strRawDEMDataFile.c_str()) );
	m_pElevDataSource->SetOffsets(m_TerrainRenderParams.m_iColOffset, m_TerrainRenderParams.m_iRowOffset);
	m_fMinElevation = m_pElevDataSource->GetGlobalMinElevation() * m_TerrainRenderParams.m_TerrainAttribs.m_fElevationScale;
	m_fMaxElevation = m_pElevDataSource->GetGlobalMaxElevation() * m_TerrainRenderParams.m_TerrainAttribs.m_fElevationScale;

	LPCTSTR strTileTexPaths[CEarthHemsiphere::NUM_TILE_TEXTURES], strNormalMapPaths[CEarthHemsiphere::NUM_TILE_TEXTURES];
	for( int iTile=0; iTile < _countof(strTileTexPaths); ++iTile ) {
		strTileTexPaths[iTile] = m_strTileTexPaths[iTile].c_str();
		strNormalMapPaths[iTile] = m_strNormalMapTexPaths[iTile].c_str();
	}
	V( m_EarthHemisphere.OnD3D11CreateDevice(m_pElevDataSource.get(), m_TerrainRenderParams, mpD3dDevice, mpContext, m_strRawDEMDataFile.c_str(), m_strMtrlMaskFile.c_str(), strTileTexPaths, strNormalMapPaths ) );
	D3D11_BUFFER_DESC CBDesc = {
			sizeof(SLightAttribs),
	        D3D11_USAGE_DYNAMIC,
	        D3D11_BIND_CONSTANT_BUFFER,
	        D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags
	        0, //UINT MiscFlags;
	        0, //UINT StructureByteStride;
	};
	V( mpD3dDevice->CreateBuffer( &CBDesc, NULL, &m_pcbLightAttribs) );

}

void scattering_intel::prepareFrame() {

}

void scattering_intel::render() {

}

void scattering_intel::cleanup() {
	delete m_pOffscreenRenderTarget;
	/*m_EarthHemisphere.OnD3D11DestroyDevice();
    ReleaseShadowMap();
    ReleaseTmpBackBuffAndDepthBuff();
    m_pLightSctrPP->OnDestroyDevice();
    SAFE_RELEASE(m_pDirectionalLightCamera);
    SAFE_RELEASE(m_pDirLightOrienationCamera);
    SAFE_RELEASE(mpCamera);
    SAFE_DELETE( mpCameraController);
    SAFE_DELETE( m_pLightController);
    m_pcbLightAttribs.Release();*/
}

bool scattering_intel::CreateShadowMap() {
	ReleaseShadowMap();
	static const bool bIs32BitShadowMap = true;
	//ShadowMap
	D3D11_TEXTURE2D_DESC ShadowMapDesc = {
        m_uiShadowMapResolution,
        m_uiShadowMapResolution,
		1,
		m_TerrainRenderParams.m_iNumShadowCascades,
		bIs32BitShadowMap ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R16_TYPELESS,
        {1,0},
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_DEPTH_STENCIL,
		0,
		0
	};

	CComPtr<ID3D11Texture2D> ptex2DShadowMap;
	V_RETURN(pd3dDevice->CreateTexture2D(&ShadowMapDesc, NULL, &ptex2DShadowMap));

	D3D11_SHADER_RESOURCE_VIEW_DESC ShadowMapSRVDesc;
    ZeroMemory( &ShadowMapSRVDesc, sizeof(ShadowMapSRVDesc) );
    ShadowMapSRVDesc.Format = bIs32BitShadowMap ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R16_UNORM;
    ShadowMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	ShadowMapSRVDesc.Texture2DArray.MostDetailedMip = 0;
	ShadowMapSRVDesc.Texture2DArray.MipLevels = 1;
    ShadowMapSRVDesc.Texture2DArray.FirstArraySlice = 0;
    ShadowMapSRVDesc.Texture2DArray.ArraySize = ShadowMapDesc.ArraySize;

    V_RETURN(pd3dDevice->CreateShaderResourceView(ptex2DShadowMap, &ShadowMapSRVDesc, &m_pShadowMapSRV));

    D3D11_DEPTH_STENCIL_VIEW_DESC ShadowMapDSVDesc;
    ZeroMemory( &ShadowMapDSVDesc, sizeof(ShadowMapDSVDesc) );
    ShadowMapDSVDesc.Format = bIs32BitShadowMap ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D16_UNORM;
    ShadowMapDSVDesc.Flags = 0;
    ShadowMapDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    ShadowMapDSVDesc.Texture2DArray.MipSlice = 0;
    ShadowMapDSVDesc.Texture2DArray.ArraySize = 1;
    m_pShadowMapDSVs.resize(ShadowMapDesc.ArraySize);
    for(UINT iArrSlice=0; iArrSlice < ShadowMapDesc.ArraySize; iArrSlice++)
    {
        ShadowMapDSVDesc.Texture2DArray.FirstArraySlice = iArrSlice;
        V_RETURN(pd3dDevice->CreateDepthStencilView(ptex2DShadowMap, &ShadowMapDSVDesc, &m_pShadowMapDSVs[iArrSlice]));
    }

    return D3D_OK;
}

void build_gui_window() const {
	ImGui::Begin( "Control Panel" );
	ImGui::Checkbox( "Enable light scattering", &m_bEnableLightScattering );
	ImGui::Checkbox( "Enable light shafts", &m_PPAttribs.m_bEnableLightShafts );
	ImGui::RadioButton( "Epipolar sampling", &m_PPAttribs.m_uiLightSctrTechnique, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "Brute force ray marching", &m_PPAttribs.m_uiLightSctrTechnique, 1 );
	ImGui::Text( "Shadow Map res" ); ImGui::SameLine();
	ImGui::RadioButton( "512*512", &m_uiShadowMapResolution, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "1024*1024", &m_uiShadowMapResolution, 1 ); ImGui::SameLine();
	ImGui::RadioButton( "2048*2048", &m_uiShadowMapResolution, 2 ); ImGui::SameLine();
	ImGui::RadioButton( "4096*4096", &m_uiShadowMapResolution, 3 );
	if( !m_PPAttribs.m_bEnableLightShafts && m_PPAttribs.m_uiSingleScatteringMode == SINGLE_SCTR_MODE_INTEGRATION )
		ImGui::SliderInt(
				"Num integration steps", &m_PPAttribs.m_uiInstrIntegralSteps, m_iMinEpipolarSlices, m_iMaxEpipolarSlices
		);
	ImGui::SliderInt(
			"Epipolar slices", &m_PPAttribs.m_uiNumEpipolarSlices, m_iMinEpipolarSlices, m_iMaxEpipolarSlices
	);
	ImGui::SliderInt(
			"Total samples in slice", &m_PPAttribs.m_uiMaxSamplesInSlice,
			m_iMinSamplesInEpipolarSlice, m_iMaxSamplesInEpipolarSlice
	);
	ImGui::SliderInt(
			"Initial sample step", &m_PPAttribs.m_uiInitialSampleStepInSlice,
			0, m_PPAttribs.m_uiMaxSamplesInSlice / m_iMinInitialSamplesInEpipolarSlice
	);
	ImGui::SliderInt(
			"Epipole sampling density", &m_PPAttribs.m_uiEpipoleSamplingDensityFactor,
			0, m_iMaxEpipoleSamplingDensityFactor
	);
	ImGui::SliderFloat( "Refinement threshold", &m_PPAttribs.m_fRefinementThreshold, 0.001f, 0.5f );
	ImGui::SliderFloat( "Scattering Scale", &m_fScatteringScale, 0.1f, 2.0f );
	ImGui::Checkbox( "Show sampling", &m_PPAttribs.m_bShowSampling );
	ImGui::Checkbox( "Optimize sample locations", &m_PPAttribs.m_bOptimizeSampleLocations );
	ImGui::Checkbox( "Correction at depth breaks", &m_PPAttribs.m_bCorrectScatteringAtDepthBreaks );
	ImGui::Checkbox( "Show depth breaks", &m_PPAttribs.m_bShowDepthBreaks );
	ImGui::Checkbox( "Lighting only", &m_PPAttribs.m_bShowLightingOnly );
	ImGui::Text( "Single scattering" ); ImGui::SameLine();
	ImGui::RadioButton( "none", &m_PPAttribs.m_uiSingleScatteringMode, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "integration", &m_PPAttribs.m_uiSingleScatteringMode, 1 ); ImGui::SameLine();
	ImGui::RadioButton( "LUT", &m_PPAttribs.m_uiSingleScatteringMode, 2 );
	ImGui::Text( "Multiple scattering" ); ImGui::SameLine();
	ImGui::RadioButton( "none", &m_PPAttribs.m_uiMultipleScatteringMode, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "unoccluded", &m_PPAttribs.m_uiMultipleScatteringMode, 1 ); ImGui::SameLine();
	ImGui::RadioButton( "occluded", &m_PPAttribs.m_uiMultipleScatteringMode, 2 );
	ImGui::SliderInt( "Num Cascades", &m_TerrainRenderParams.m_iNumShadowCascades, 1, MAX_CASCADES-1 );
	// @todo ?
	bool of_whatever{ false };
	ImGui::Checkbox( "Show cascades", &of_whatever );
	ImGui::Checkbox( "Smooth shadows", &m_TerrainRenderParams.m_bSmoothShadows );
	ImGui::Checkbox( "Best cascade search", &m_TerrainRenderParams.m_bBestCascadeSearch );
	ImGui::SliderFloat( "Partitioning Factor", &m_fCascadePartitioningFactor, 0.0f, 1.0f );
	ImGui::Text( "Cascade processing" ); ImGui::SameLine();
	ImGui::RadioButton( "single pass", &m_PPAttribs.m_uiCascadeProcessingMode, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "multipass", &m_PPAttribs.m_uiCascadeProcessingMode, 1 ); ImGui::SameLine();
	ImGui::RadioButton( "multipass inst", &m_PPAttribs.m_uiCascadeProcessingMode, 2 );
	ImGui::SliderInt( "First cascade to ray march", &m_PPAttribs.m_iFirstCascade, 0, MAX_CASCADES-1 );
	ImGui::Text( "Extinction eval mode" ); ImGui::SameLine();
	ImGui::RadioButton( "per pixel", &m_PPAttribs.m_uiExtinctionEvalMode, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "epipolar", &m_PPAttribs.m_uiExtinctionEvalMode, 1 );
	ImGui::Text( "Refinement criterion" ); ImGui::SameLine();
	ImGui::RadioButton( "depth", &m_PPAttribs.m_uiRefinementCriterion, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "inscattering", &m_PPAttribs.m_uiRefinementCriterion, 1 );
	ImGui::Text( "Min/max format" ); ImGui::SameLine();
	ImGui::RadioButton( "16u", &m_PPAttribs.m_bIs32BitMinMaxMipMap, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "32f", &m_PPAttribs.m_bIs32BitMinMaxMipMap, 1 );
	ImGui::Checkbox( "Use custom sctr coeffs", &m_PPAttribs.m_bUseCustomSctrCoeffs );
	omath::vec3 reyleigh_color;
	omath::vec3 mie_color;
	// @todo ?
	if( m_PPAttribs.m_bUseCustomSctrCoeffs ) {
		ImGui::ColorPicker3( "Rayleigh color", &rayleigh_color, ImGuiColorEditFlags_RGB, NULL );
		ImGui::ColorPicker3( "Set Mie color", &mie_color, ImGuiColorEditFlags_RGB, NULL );
	}
	ImGui::SliderFloat( "Aerosol density", &m_PPAttribs.m_fAerosolDensityScale, 0.1f, 5.0f );
	ImGui::SliderFloat( "Aerosol absorbtion", &m_PPAttribs.m_fAerosolAbsorbtionScale, 0.0f, 5.0f );
	ImGui::Checkbox( "Animate sun", &m_bAnimateSun );
	ImGui::SliderFloat( "Middle gray (Key)", &m_PPAttribs.m_fMiddleGray, 0.01f, 1.0f );
	if( m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_MODE_REINHARD_MOD ||
		m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_MODE_UNCHARTED2 ||
		m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_LOGARITHMIC ||
		m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_ADAPTIVE_LOG )
		ImGui::SliderFloat( "White point", &m_PPAttribs.m_fWhitePoint, 0.01f, 10.0f );
	if( m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_MODE_EXP ||
		m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_MODE_REINHARD ||
		m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_MODE_REINHARD_MOD ||
		m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_LOGARITHMIC ||
		m_PPAttribs.m_uiToneMappingMode == TONE_MAPPING_ADAPTIVE_LOG )
		ImGui::SliderFloat( "Luminance saturation", &m_PPAttribs.m_fLuminanceSaturation, 0.01f, 2.0f );
	ImGui::Checkbox( "Auto exposure", &m_PPAttribs.m_bAutoExposure );
	ImGui::Text( "Tone mapping" ); ImGui::SameLine();
	ImGui::RadioButton( "exp", &m_PPAttribs.m_uiToneMappingMode, 0 ); ImGui::SameLine();
	ImGui::RadioButton( "Reinhard", &m_PPAttribs.m_uiToneMappingMode, 1 ); ImGui::SameLine();
	ImGui::RadioButton( "Reinhard Mod", &m_PPAttribs.m_uiToneMappingMode, 2 );
	ImGui::RadioButton( "Uncharted 2", &m_PPAttribs.m_uiToneMappingMode, 3 ); ImGui::SameLine();
	ImGui::RadioButton( "Filmic ALU", &m_PPAttribs.m_uiToneMappingMode, 4 ); ImGui::SameLine();
	ImGui::RadioButton( "Logarithmic", &m_PPAttribs.m_uiToneMappingMode, 5 ); ImGui::SameLine();
	ImGui::RadioButton( "Adaptive log", &m_PPAttribs.m_uiToneMappingMode, 6 );
	if( m_PPAttribs.m_bAutoExposure )
		ImGui::Checkbox( "Light adaptation", &m_PPAttribs.m_bLightAdaptation );
	ImGui::Text( "F1 for Help" );
	ImGui::Text( "[Escape] to quit application" );
	ImGui::Text( "A,S,D,F - move camera position" );
	ImGui::Text( "Q - camera position down" );
	ImGui::Text( "E - camera position up" );
	ImGui::Text( "[Shift] - accelerate camera movement" );
	ImGui::Text( "mouse + left click - camera look rotation" );
	ImGui::Text( "mouse + right click - light rotation" );
	ImGui::End();
}

}
