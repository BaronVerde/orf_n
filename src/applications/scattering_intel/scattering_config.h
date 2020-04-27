
#pragma once

#include <string>

namespace scattering {

const std::string RawDEMDataFile = "media\Terrain\HeightMap.tif";
const std::string MaterialMaskFile = "media\Terrain\Mask.png";
//const TexturingMode = MaterialMaskNM;
const int ElevationSamplingInterval = 32;
const int ScalingFactor = 10;
const int RingDimension = 65;
const int NumRings = 15;
const int ColOffset = 1356;
const int RowOffset = 924;
const std::string TileTexture0 = "media\tiles\gravel_DM.dds";
const std::string TileTexture1 = "media\tiles\grass_DM.dds";
const std::string TileTexture2 = "media\tiles\cliff_DM.dds";
const std::string TileTexture3 = "media\tiles\snow_DM.dds";
const std::string TileTexture4 = "media\tiles\grassDark_DM.dds";
const std::string TileNormalMap0 = "media\tiles\gravel_NM.dds";
const std::string TileNormalMap1 = "media\tiles\grass_NM.dds";
const std::string TileNormalMap2 = "media\tiles\cliff_NM.dds";
const std::string TileNormalMap3 = "media\tiles\Snow_NM.jpg";
const std::string TileNormalMap4 = "media\tiles\grass_NM.dds";
const int TilingScale0 = 200;
const int TilingScale1 = 500;
const int TilingScale2 = 800;
const int TilingScale3 = 80;
const int TilingScale4 = 80;
const bool AnimateSun = false;

const int m_iMinEpipolarSlices = 32;
const int m_iMaxEpipolarSlices = 2048;
const int m_iMinSamplesInEpipolarSlice = 32;
const int m_iMaxSamplesInEpipolarSlice = 2048;
const int m_iMaxEpipoleSamplingDensityFactor = 32;
const int m_iMinInitialSamplesInEpipolarSlice = 8;

}
