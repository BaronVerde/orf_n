
#pragma once

#include <applications/camera/camera.h>
#include <applications/TerrainLOD/HeightMap.h>
#include <string>

namespace terrain {

/**
 * No more than 15 lod levels !
 */
static const int NUMBER_OF_LOD_LEVELS{ 6 };

// @todo: Is there a connection at all ?
static const int NUMBER_OF_GRID_MESHES{ NUMBER_OF_LOD_LEVELS + 1 };

// @todo should depend on node size and lod levels
static const int MAX_NUMBER_SELECTED_NODES{ 1024 };

// @todo: calc from number of lod levels and heightmap size. Memory usage rises for small nodes.
// Must be power of 2.
static const int LEAF_NODE_SIZE{ 32 };

// Size x/z of terrain tiles
// @todo: in a future version this could be handled dynamically,
// also perform check on heightmap loading !
static const omath::uvec2 TILE_SIZE{ 2048, 2048 };

/**
 * Determines rendering LOD level distribution based on distance from the viewer.
 * Value of 2.0 should result in equal number of triangles displayed on screen (in
 * average) for all distances. Values above 2.0 will result in more triangles
 * on more distant areas, and vice versa.
 * Must be between 1.5 and 16.0 !
 */
static const float LOD_LEVEL_DISTANCE_RATIO{ 1.8f };

// [0, 1] when to start morphing to the next (lower-detailed) LOD level;
// default is 0.67 - first 0.67 part will not be morphed, and the morph will go from 0.67 to 1.0
static const float MORPH_START_RATIO{ 0.8f };

// 1:1 texel to grid ratio
static const int RENDER_GRID_RESULUTION_MULT{ 1 };

static const int GRIDMESH_DIMENSION{ LEAF_NODE_SIZE * RENDER_GRID_RESULUTION_MULT };

}
