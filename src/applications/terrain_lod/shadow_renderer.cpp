
#include "shadow_renderer.h"
#include "sky.h"
#include "settings.h"

namespace terrain {

cascaded_shadow_map::cascaded_shadow_map() {
	m_settings.flags = SF_IgnoreZ;
	m_layers_array = new layer*[NUMBER_OF_LOD_LEVELS];
	for( int i = 0; i < NUMBER_OF_LOD_LEVELS; ++i )
		m_layers_array[i] = &m_cascades[i];
}

cascaded_shadow_map::~cascaded_shadow_map() {
	delete [] m_layers_array;
}

void cascaded_shadow_map::cleanup() {
	for( int i = 0; i < NUMBER_OF_LOD_LEVELS; ++i )
		glDeleteRenderbuffers( 1, &m_cascades[i].ShadowMapDepth );
	glDeleteFramebuffers( 1, &m_scratchSurface );
}

void cascaded_shadow_map::setup() {
	if( m_defPoolTexturesCreated )
		return;
	m_defPoolTexturesCreated = true;
	// start with a smooth big 6.0 radius
	float samplingRadius = (float)SHADOW_MAP_RESOLUTION / 384.0f;
	// reduce radius by this amount for each level - 2.0 would be correct,
	// but less is good enough while still retaining smoothnes on higher levels
	const float samplingRadiusStep = 1.75f;
	glCreateFramebuffers( 1, &m_scratchSurface );
	for( int i = 0; i < NUMBER_OF_LOD_LEVELS; ++i ) {
		glCreateRenderbuffers( 1, &m_cascades[i].ShadowMapDepth );
		glNamedRenderbufferStorage(
				m_cascades[i].ShadowMapDepth, GL_R32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION
		);
		// calculate sampling radius so that the transition between layers is smooth
		m_cascades[i].SamplingTexelRadius = std::max( 1.1f, samplingRadius );
		samplingRadius /= samplingRadiusStep;
		// check completeness
		glNamedFramebufferRenderbuffer(
				m_scratchSurface, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_cascades[i].ShadowMapDepth
		);
		if( GL_FRAMEBUFFER_COMPLETE != glCheckNamedFramebufferStatus( m_scratchSurface, GL_DRAW_FRAMEBUFFER ) )
			std::cerr << "Error creating csm framebuffer. Framebuffer incomplete." << std::endl;
	}
	cascaded_volume_map::reset();
}

void cascaded_shadow_map::UpdateShaderSettings( D3DXMACRO * newMacroDefines ) {
	if( m_mainRenderer == NULL )
		return;
	// copy and add our own
	D3DXMACRO macros[16] = { NULL };
	int i;
	for( i = 0; i < 14; i++ ) {
		if( newMacroDefines[i].Name == NULL )
			break;
		macros[i].Name = newMacroDefines[i].Name;
		macros[i].Definition = newMacroDefines[i].Definition;
	}
	macros[i].Name       = "IS_GENERATING_SHADOWS";
	macros[i].Definition = "1";
	macros[i].Name       = NULL;
	macros[i].Definition = NULL;
	m_vsShadow.SetShaderInfo( "Shaders/CDLODTerrain.vsh", "terrainShadow", macros );
	m_psShadow.SetShaderInfo( "Shaders/misc.psh", "writeShadow", macros );
}

struct ShadowRenderContext {
   IDirect3DDevice9 *                  D3DDevice;
   DemoCamera *                        Camera;
   DemoSky *                           Sky;
   int                                 GridMeshDim;
   bool                                UseDetailHeightmap;
   const CDLODQuadTree::LODSelection *  TerrainDLODSelection;
   CDLODRenderStats *                   RenderStats;
   const CDLODRenderer *                DlodRenderer;
   const CDLODQuadTree *                TerrainQuadTree;
   MapDimensions                       MapDims;
   IDirect3DTexture9 *                 TerrainHMTexture;
   IDirect3DTexture9 *                 TerrainDetailHMTexture;
   IDirect3DSurface9 *                 ScratchSurface;
   D3DXVECTOR3                         ShadowForward;
   D3DXVECTOR3                         ShadowRight;
   D3DXVECTOR3                         ShadowUp;
   DxVertexShader *                    VSShadow;
   DxPixelShader *                     PSShadow;
   const DemoRenderer *                MainRenderer;
};
//
bool cascaded_shadow_map::CSMLayer::RenderShadowMap( void * _renderContext, CSMLayer * parentLayer )
{
   ShadowRenderContext & renderContext = *((ShadowRenderContext*)_renderContext);
   
   if( ShadowMapDepth == NULL || renderContext.ScratchSurface == NULL )
	   return true;

   HRESULT hr;
   IDirect3DDevice9* device = renderContext.D3DDevice;

   if( ShadowMap != NULL )
   {
      vaSetRenderTargetTexture( 0, ShadowMap );
      V( device->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_ALPHA ) );
   }
   else
   {
      device->SetRenderTarget( 0, renderContext.ScratchSurface );
      V( device->SetRenderState( D3DRS_COLORWRITEENABLE, 0x00 ) );
   }
   vaSetDepthStencilTexture( ShadowMapDepth );

   // Clear the render target and depth buffer (or just the depth buffer, depending on the technique)
   V( device->Clear( 0, NULL, (ShadowMap != NULL)?(D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET):(D3DCLEAR_ZBUFFER), D3DCOLOR_ARGB( 255, 255, 255, 255 ), 1.0f, 0 ) );

   int ShadowmapTextureResolution;
   {
      D3DSURFACE_DESC surfDesc;
      ShadowMapDepth->GetLevelDesc(0, &surfDesc);
      ShadowmapTextureResolution = surfDesc.Width;
      assert( surfDesc.Width == surfDesc.Height );
   }

   //////////////////////////////////////////////////////////////////////////
   // calculate shadow camera
   {
      struct OBB
      {
         float MinX;
         float MinY;
         float MinZ;
         float MaxX;
         float MaxY;
         float MaxZ;
      };

      OBB obb;
      obb.MinX = FLT_MAX;
      obb.MinY = FLT_MAX;
      obb.MinZ = FLT_MAX;
      obb.MaxX = -FLT_MAX;
      obb.MaxY = -FLT_MAX;
      obb.MaxZ = -FLT_MAX;

      // calculate shadow coverage - our cascade bounding box
      AABB boundingBox( this->BoxMin, this->BoxMax );
      D3DXVECTOR3 boundingBoxSize = boundingBox.Size();
      {
         D3DXVECTOR3 corners[8];
         boundingBox.GetCornerPoints(corners);

         for( int j = 0; j < 8; j++ )
         {
            float dx = D3DXVec3Dot( &renderContext.ShadowRight, &corners[j] );
            float dy = D3DXVec3Dot( &renderContext.ShadowUp, &corners[j] );
            float dz = D3DXVec3Dot( &renderContext.ShadowForward, &corners[j] );

            obb.MinX = ::min( obb.MinX, dx );
            obb.MinY = ::min( obb.MinY, dy );
            obb.MinZ = ::min( obb.MinZ, dz );
            obb.MaxX = ::max( obb.MaxX, dx );
            obb.MaxY = ::max( obb.MaxY, dy );
            obb.MaxZ = ::max( obb.MaxZ, dz );
         }
      }

      D3DXVECTOR3 boundingBoxSizeNoZ( boundingBoxSize.x, boundingBoxSize.y, 0.0f );
      float cascadeLODLevelDiameterNoZ = D3DXVec3Length( &boundingBoxSizeNoZ );

      D3DXVECTOR3 shadowCamEye = ((obb.MinX + obb.MaxX) * 0.5f) * renderContext.ShadowRight + ((obb.MinY + obb.MaxY) * 0.5f) * renderContext.ShadowUp + (obb.MinZ) * renderContext.ShadowForward;

      //float obbWidth    = obb.MaxX - obb.MinX;
      //float obbHeight   = obb.MaxY - obb.MinY;
      float obbDepth    = obb.MaxZ - obb.MinZ;

      // make the shadow map cover area fixed to help eliminate precision and flickering problems
      float obbRealWidth = cascadeLODLevelDiameterNoZ * 1.01f;
      float obbRealHeight = cascadeLODLevelDiameterNoZ * 1.01f;
      //assert( obbRealWidth >= obbWidth );
      //assert( obbRealHeight >= obbHeight );

      this->WorldSpaceTexelSizeX = obbRealWidth / (float)ShadowmapTextureResolution;
      this->WorldSpaceTexelSizeY = obbRealHeight / (float)ShadowmapTextureResolution;

      float worldSpaceTexelSizeXY = ::max( WorldSpaceTexelSizeX, WorldSpaceTexelSizeY );
      this->NoiseScaleX             = (renderContext.MapDims.SizeX / worldSpaceTexelSizeXY);
      this->NoiseScaleY             = (renderContext.MapDims.SizeY / worldSpaceTexelSizeXY);
      //this->NoiseDepthOffsetScale   = currentCascade.DepthOffset;

      // this was used to control shadow step to prevent flickering - not used anymore since now each cascade is
      // refreshed in steps and is not rendered continuously (every frame) as before
      float allowedShadowCamStep = ::max( WorldSpaceTexelSizeX, WorldSpaceTexelSizeY ) * 2.0f;
      float mx = D3DXVec3Dot( &shadowCamEye, &renderContext.ShadowRight );
      float my = D3DXVec3Dot( &shadowCamEye, &renderContext.ShadowUp );
      float mz = D3DXVec3Dot( &shadowCamEye, &renderContext.ShadowForward );
      mx -= fmodf( mx, allowedShadowCamStep ) + allowedShadowCamStep * 0.5f;
      my -= fmodf( my, allowedShadowCamStep ) + allowedShadowCamStep * 0.5f;
      mz -= fmodf( mz, allowedShadowCamStep ) + allowedShadowCamStep * 0.5f;
      shadowCamEye = mx * renderContext.ShadowRight + my * renderContext.ShadowUp + mz * renderContext.ShadowForward;

      // A pile of hacks be here.
      float maxShadowLengthK = tanf( ::min( acosf( ::min( 1.0f, -renderContext.ShadowForward.z ) ), 1.4f ) );
      const float neighbourOffsetEpsilon = maxShadowLengthK * renderContext.TerrainQuadTree->GetWorldMapDims().SizeZ * 1.0f;
      shadowCamEye -= renderContext.ShadowForward * neighbourOffsetEpsilon;
      obbDepth += neighbourOffsetEpsilon;

      float biasMultiplier = this->SamplingTexelRadius * this->ApproxWorldTexelSize / obbDepth;
      float fDepthBias = 4.0f * biasMultiplier;
      float fBiasSlope = 400.0f * biasMultiplier;

      //depth bias
      device->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fDepthBias);
      device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&fBiasSlope);

      // used for z-noise, offsets or anything similar
      this->ApproxWorldTexelSize = ::max( WorldSpaceTexelSizeX, WorldSpaceTexelSizeY );

      D3DXMATRIX view;
      D3DXMATRIX proj;

      D3DXVECTOR3 shadowCamLookAt = shadowCamEye + renderContext.ShadowForward;
      D3DXMatrixLookAtLH( &view, &shadowCamEye, &shadowCamLookAt, &renderContext.ShadowUp );
      D3DXMatrixOrthoLH( &proj, obbRealWidth, obbRealHeight, 0, obbDepth );
      
      this->ShadowView     = view;
      this->ShadowProj     = proj;
      this->ShadowViewProj = view * proj;

      this->OrthoEyePos       = shadowCamEye;
      this->OrthoProjWidth    = obbRealWidth;
      this->OrthoProjHeight   = obbRealHeight;
      this->OrthoProjDepth    = obbDepth;
      
      V( renderContext.VSShadow->SetMatrix( "g_shadowViewProjection", this->ShadowViewProj ) );
   }

   D3DXPLANE planes[6];
   vaGetFrustumPlanes(planes, this->ShadowViewProj);


   // and then make the new selection that we'll use to render shadow map
   CDLODQuadTree::LODSelectionOnStack<4096>     cdlodSelection( renderContext.Camera->GetPosition(), renderContext.Camera->GetViewRange() * 0.95f, planes, renderContext.TerrainDLODSelection->GetLODDistanceRatio() );
   renderContext.TerrainQuadTree->LODSelect( &cdlodSelection );
   // 
   //////////////////////////////////////////////////////////////////////////

   // This contains all settings used to do rendering through CDLODRenderer
   CDLODRendererBatchInfo cdlodBatchInfo;

   cdlodBatchInfo.VertexShader          = renderContext.VSShadow;
   cdlodBatchInfo.VSGridDimHandle       = renderContext.VSShadow->GetConstantTable()->GetConstantByName( NULL, "g_gridDim" );
   cdlodBatchInfo.VSQuadScaleHandle     = renderContext.VSShadow->GetConstantTable()->GetConstantByName( NULL, "g_quadScale" );
   cdlodBatchInfo.VSQuadOffsetHandle    = renderContext.VSShadow->GetConstantTable()->GetConstantByName( NULL, "g_quadOffset" );
   cdlodBatchInfo.VSMorphConstsHandle   = renderContext.VSShadow->GetConstantTable()->GetConstantByName( NULL, "g_morphConsts" );
   cdlodBatchInfo.VSUseDetailMapHandle  = renderContext.VSShadow->GetConstantTable()->GetConstantByName( NULL, "g_useDetailMap" );
   cdlodBatchInfo.MeshGridDimensions    = renderContext.GridMeshDim;
   cdlodBatchInfo.DetailMeshLODLevelsAffected = 0;

   //assert( cdlodBatchInfo.VSGridDimHandle != NULL );

   //////////////////////////////////////////////////////////////////////////
   // Setup global shader settings
   //
   renderContext.DlodRenderer->SetIndependentGlobalVertexShaderConsts( *renderContext.VSShadow, *renderContext.TerrainQuadTree, this->ShadowViewProj, renderContext.Camera->GetPosition() );
   //
   const DemoRenderer::Settings & mainSettings = renderContext.MainRenderer->GetSettings();
   //
   // setup detail heightmap globals if any
   if( mainSettings.DetailHeightmapEnabled )
   {
      V( renderContext.VSShadow->SetTexture( "g_detailHMVertexTexture", renderContext.TerrainDetailHMTexture, D3DTADDRESS_WRAP, D3DTADDRESS_WRAP, D3DTEXF_LINEAR, D3DTEXF_LINEAR ) );

      D3DSURFACE_DESC desc;
      renderContext.TerrainDetailHMTexture->GetLevelDesc( 0, &desc );

      int dmWidth = desc.Width;
      int dmHeight = desc.Height;

      float sizeX = (renderContext.MapDims.SizeX / (float)renderContext.TerrainQuadTree->GetRasterSizeX()) * dmWidth / mainSettings.DetailHeightmapXYScale;
      float sizeY = (renderContext.MapDims.SizeY / (float)renderContext.TerrainQuadTree->GetRasterSizeY()) * dmHeight / mainSettings.DetailHeightmapXYScale;

      cdlodBatchInfo.DetailMeshLODLevelsAffected = mainSettings.DetailMeshLODLevelsAffected;

      renderContext.VSShadow->SetFloatArray( "g_detailConsts", sizeX, sizeY, mainSettings.DetailHeightmapZSize, (float)mainSettings.DetailMeshLODLevelsAffected );
   }
   //
   // vertex textures
   renderContext.VSShadow->SetTexture( "g_terrainHMVertexTexture", renderContext.TerrainHMTexture, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTEXF_LINEAR, D3DTEXF_LINEAR );
   //m_vsShadow.SetTexture( "g_terrainNMVertexTexture", m_terrainNMTexture, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTEXF_LINEAR, D3DTEXF_LINEAR );
   //
   // end of global shader settings
   //////////////////////////////////////////////////////////////////////////

   //////////////////////////////////////////////////////////////////////////
   // connect quad selection to our render batch info
   cdlodBatchInfo.CDLODSelection = &cdlodSelection;
   //////////////////////////////////////////////////////////////////////////

   //////////////////////////////////////////////////////////////////////////
   // Render
   //
   // Clear the render target and depth buffer 
   cdlodBatchInfo.FilterLODLevel  = -1;
   cdlodBatchInfo.PixelShader     = renderContext.PSShadow;
   CDLODRenderStats stepStats;
   renderContext.DlodRenderer->Render( cdlodBatchInfo, &stepStats );
   if( renderContext.RenderStats != NULL )
      renderContext.RenderStats->Add( stepStats );
   //
   //////////////////////////////////////////////////////////////////////////

   return !cdlodSelection.IsVisDistTooSmall();
}
//
bool cascaded_shadow_map::CSMLayer::Update( const D3DXVECTOR3 & observerPos, float newVisibilityRange, CascadedVolumeMap * parent, bool forceUpdate )
{
   if( !Layer::Update( observerPos, newVisibilityRange, parent, forceUpdate ) )
      return false;

   TaggedForRefresh = true;
   TaggedForRefreshTime = parent->GetCurrentTime();

   return true;
}
//
HRESULT cascaded_shadow_map::Render( float deltaTime, DemoCamera * camera, DemoSky * lightMgr, int gridMeshDim, bool useDetailHeightmap, const CDLODQuadTree::LODSelection & terrainDLODSelection, CDLODRenderStats * renderStats )
{
   HRESULT hr;
   IDirect3DDevice9* device = GetD3DDevice();

#ifdef MY_EXTENDED_STUFF
   Prof(DemoShadowsRenderer_Render);
#endif

   //gridMeshDim /= 2;

   InitializeRuntimeData();

   bool lightChanged = false;

   //////////////////////////////////////////////////////////////////////////
   // calculate shadow camera forward/right/up
   // If light has moved, recalculate new light direction and reset cascades
   if( D3DXVec3Dot( &m_shadowForward, &lightMgr->GetDirectionalLightDir() ) < (1.0f - 1e-5f) )
   {
      m_shadowUp = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
      m_shadowForward = lightMgr->GetDirectionalLightDir();

      D3DXVec3Normalize( &m_shadowForward, &m_shadowForward );

      // special case for near horizontal - keep previous frame's one to prevent flicker
      if( fabsf( D3DXVec3Dot( &m_shadowForward, &m_shadowUp ) ) > 0.99f )
         D3DXVec3Cross( &m_shadowUp, &m_lastShadowCamRightVec, &m_shadowForward );

      D3DXVec3Cross( &m_shadowRight, &m_shadowForward, &m_shadowUp );
      D3DXVec3Normalize( &m_shadowRight, &m_shadowRight );
      D3DXVec3Cross( &m_shadowUp, &m_shadowRight, &m_shadowForward );
      D3DXVec3Normalize( &m_shadowUp, &m_shadowUp );

      for( int i = m_layerCount-1; i >= 0; i-- )
      {
         CSMLayer & layer = m_cascades[i];
         layer.BoxMax = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
         layer.BoxMin = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
      }

      m_lastShadowCamRightVec = m_shadowRight;

      lightChanged = true;
   }
   //////////////////////////////////////////////////////////////////////////

   if( renderStats != NULL )
      renderStats->Reset();

   if( lightChanged )
   {
      CascadedVolumeMap::Reset();
      m_settings.AllowedUpdatesPerFrame = 5.0f;
   }
   else
   {
      m_settings.AllowedUpdatesPerFrame = 1.0f;
   }

   int numberUpdated = CascadedVolumeMap::Update( deltaTime, camera->GetPosition(), terrainDLODSelection.GetLODLevelRanges() );

   // if there's nothing to do get out now
   if( numberUpdated == 0 )
   {
      // lazy version:
       //return S_OK;

      // try reducing one type of flicker by always updating at least one every second 
      // frame, but only if there are no other updates:
      static int stepK = 0;
      stepK++;
      if( (stepK % ((m_mainRenderer->GetSettings().ShadowmapHighQuality)?(2):(3))) != 0 )
         return S_OK;

      int leastFreshIndex = 0;
      double leastFreshTime = m_cascades[0].TaggedForRefreshTime;
      for( int i = 0+1; i < m_layerCount; i++ )
      {
         if( m_cascades[i].TaggedForRefreshTime < leastFreshTime )
         {
            leastFreshTime = m_cascades[i].TaggedForRefreshTime;
            leastFreshIndex = i;
         }
      }

      m_cascades[leastFreshIndex].Update( camera->GetPosition(), terrainDLODSelection.GetLODLevelRanges()[leastFreshIndex], this, true );
      numberUpdated = 1;
   }

   //save old viewport
   D3DVIEWPORT9 oldViewport;
   GetD3DDevice()->GetViewport(&oldViewport);

   D3DVIEWPORT9 newViewport;
   newViewport.X = 0;
   newViewport.Y = 0;
   newViewport.Width  = m_textureResolution;
   newViewport.Height = m_textureResolution;
   newViewport.MinZ = 0.0f;
   newViewport.MaxZ = 1.0f;
   GetD3DDevice()->SetViewport(&newViewport);

   // Backup DirectX state
   IDirect3DSurface9 * oldRT = NULL;
   IDirect3DSurface9 * oldDepthStencil = NULL;
   V( device->GetRenderTarget( 0, &oldRT ) );
   V( device->GetDepthStencilSurface( &oldDepthStencil ) );

   //V( device->SetDepthStencilSurface( m_depthBuffer ) );
   //V( device->SetRenderTarget( 0, NULL ) );

   device->SetRenderState( D3DRS_ZENABLE, TRUE );
   device->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
   device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

   V( device->SetVertexShader( m_vsShadow ) );
   V( device->SetPixelShader( m_psShadow ) );

   bool visDistanceTooSmall = false;


   ShadowRenderContext src;
   src.Camera                 = camera;
   src.Sky                    = lightMgr;
   src.GridMeshDim            = gridMeshDim;
   src.TerrainDLODSelection   = &terrainDLODSelection;
   src.RenderStats            = renderStats;
   src.UseDetailHeightmap     = useDetailHeightmap;
   src.DlodRenderer           = &m_mainRenderer->GetDLODRenderer();
   src.TerrainQuadTree        = &m_mainRenderer->GetTerrainQuadTree();
   src.MapDims                = src.TerrainQuadTree->GetWorldMapDims();
   src.TerrainHMTexture       = (IDirect3DTexture9 *)m_mainRenderer->GetTerrainHMTexture();
   src.TerrainDetailHMTexture = (IDirect3DTexture9 *)m_mainRenderer->GetTerrainDetailHMTexture();
   src.ScratchSurface         = m_scratchSurface;
   src.D3DDevice              = GetD3DDevice();
   src.ShadowForward          = m_shadowForward;
   src.ShadowRight            = m_shadowRight;
   src.ShadowUp               = m_shadowUp;
   src.VSShadow               = &m_vsShadow;
   src.PSShadow               = &m_psShadow;
   src.MainRenderer           = m_mainRenderer;

   for( int i = m_layerCount-1; i >= 0; i-- )
   {
      CSMLayer & layer = m_cascades[i];

      //// TODO: remove '|| i == 0'
      //if( layer.TaggedForRefresh || i == 0 )
      if( layer.TaggedForRefresh )
      {
         src.TerrainQuadTree->GetAreaMinMaxHeight( layer.BoxMin.x, layer.BoxMin.y, layer.BoxMax.x - layer.BoxMin.x, layer.BoxMax.y - layer.BoxMin.y, layer.BoxMin.z, layer.BoxMax.z );

         // TODO:
         // should also make sure that shadowmap is not recreated if out of Z range but I'll leave that out for now 
         // make that system somehow universal because it will be used for cascaded wave maps too

         layer.RenderShadowMap( &src, (i == (m_layerCount-1))?(NULL):(&m_cascades[i+1]) );
         layer.TaggedForRefresh = false;
      }
   }

   //const CDLODQuadTree & terrainQuadTree      = m_mainRenderer->GetTerrainQuadTree();
   //for( int i = 0; i < m_cascadeCount; i++ )
   //{
   //   //device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
   //   bool result = RenderCascade( i, camera, gridMeshDim, useDetailHeightmap, terrainDLODSelection, renderStats );
   //   visDistanceTooSmall = visDistanceTooSmall || !result;
   //   //device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
   //}

   // Check if we have too small visibility distance that causes morph between LOD levels to be incorrect.
   if( visDistanceTooSmall )
   {
      GetCanvas2D()->DrawString( GetBackbufferSurfaceDesc().Width/2 - 192, GetBackbufferSurfaceDesc().Height/2 + 16, 0xFFFF4040, L"Visibility distance might too low for LOD morph (in shadow mapping) to work correctly!" );
   }
 
 
   V( device->SetStreamSource(0, NULL, 0, 0 ) );
   V( device->SetIndices(NULL) );

   V( device->SetVertexShader( NULL ) );
   V( device->SetPixelShader( NULL ) );

   V( device->SetDepthStencilSurface( oldDepthStencil ) );
   V( device->SetRenderTarget( 0, oldRT ) );
 
   device->SetRenderState( D3DRS_ZENABLE, TRUE );
   device->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
   device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

   V( device->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED ) );

   SAFE_RELEASE( oldRT );
   SAFE_RELEASE( oldDepthStencil );

   GetD3DDevice()->SetViewport(&oldViewport);

   //depth bias
   float fTemp = 0.0f;
   GetD3DDevice()->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fTemp);
   GetD3DDevice()->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&fTemp);

   return S_OK;
}

}
