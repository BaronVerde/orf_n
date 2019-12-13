
#include <applications/Camera/Camera.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/zBackup_SinglePrecision/LODSelection.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/zBackup_SinglePrecision/Node.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/zBackup_SinglePrecision/QuadTree.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/zBackup_SinglePrecision/StreamingTerrain.h>
#include <applications/TerrainLOD/TerrainLOD.backup.latest/zBackup_SinglePrecision/TerrainTile.h>
#include <base/Globals.h>
#include <base/Logbook.h>
#include <geometry/AABB.h>
#include <omath/mat4.h>
#include <renderer/IndexBuffer.h>
#include <renderer/Module.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>

#include "Terrain/GridMesh.h"

StreamingTerrain::StreamingTerrain() : Renderable( "StreamingTerrain" ) {
	// Prepare and check settings
	if( !omath::isPowerOf2( terrain::LEAF_NODE_SIZE ) ||
			terrain::LEAF_NODE_SIZE < 2 || terrain::LEAF_NODE_SIZE > 1024 ) {
		std::string s{ "Settings LEAF_NODE_SIZE must be power of 2 and between 2 and 1024." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
	}
	if( !omath::isPowerOf2( terrain::RENDER_GRID_RESULUTION_MULT ) || terrain::RENDER_GRID_RESULUTION_MULT < 1 ) {
		std::string s{ "Settings RENDER_GRID_RESULUTION_MULT must be power of 2 and between 1 and LEAF_NODE_SIZE." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	if( terrain::NUMBER_OF_LOD_LEVELS < 2 || terrain::NUMBER_OF_LOD_LEVELS > 15 ) {
		std::string s{ "Settings NUMBER_OF_LOD_LEVELS must be between 1 and 15." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
	}
	if( terrain::LOD_LEVEL_DISTANCE_RATIO < 1.5f || terrain::LOD_LEVEL_DISTANCE_RATIO > 16.0f ) {
		std::string s{ "Settings LOD_LEVEL_DISTANCE_RATIO must be between 1.5f and 16.0f." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
	}
	if( !omath::isPowerOf2( terrain::GRIDMESH_DIMENSION ) ||
			terrain::GRIDMESH_DIMENSION < 8 || terrain::GRIDMESH_DIMENSION > 1024 ) {
		std::string s{ "Gridmesh dimension must be power of 2 and > 8 and < 1024." };
		orf_n::Logbook::getInstance().logMsg( orf_n::Logbook::TERRAIN, orf_n::Logbook::WARNING, s );
	}

	// Prepare gridmesh for drawing and load terrain tiles
	m_drawGridMesh = new terrain::GridMesh{ terrain::GRIDMESH_DIMENSION };
	m_terrainTiles.resize( terrain::MAX_NUMBER_OF_TILES );
	for( int i{0}; i < terrain::MAX_NUMBER_OF_TILES; ++i )
		m_terrainTiles[i] = new terrain::TerrainTile{ TERRAIN_FILES[i] };

	// Create terrain shaders
	std::vector<std::shared_ptr<orf_n::Module>> modules;
	modules.push_back( std::make_shared<orf_n::Module>( GL_VERTEX_SHADER,
			"Source/Applications/StreamingTerrain/Terrain.vert.glsl" ) );
	modules.push_back( std::make_shared<orf_n::Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/StreamingTerrain/Terrain.frag.glsl" ) );
	m_shaderTerrain = new orf_n::Program{ modules };

}

StreamingTerrain::~StreamingTerrain() {
	delete m_shaderTerrain;
	for( size_t i{0}; i < terrain::MAX_NUMBER_OF_TILES; ++i )
		delete m_terrainTiles[i];
	delete m_drawGridMesh;
}

void StreamingTerrain::setup() {
	// Camera and selection object. Are connected because selection is based on view frustum and range.
	// @todo parametrize or calculate initial position, direction and view range
	m_scene->getCamera()->setPositionAndTarget( { 2048.0f, 100.0f, 0.0f }, { 2048.0f, 0.0f, 2048.0f } );
	//m_scene->getCamera()->setPositionAndTarget( m_quadTree->getWholeMapBoundingBox()->getMin(), m_quadTree->getWholeMapBoundingBox()->getCenter() );
	m_scene->getCamera()->setNearPlane( 10.0f );
	m_scene->getCamera()->setFarPlane( 1000.0f );
	m_scene->getCamera()->calculateFOV();
	// @todo: no sorting for now. Must be sorted by tileIndex, distanceToCamera and lodLevel
	// to avoid too many heightmap switches and shader uniform settings.
	m_lodSelection = new terrain::LODSelection{ m_scene->getCamera(), false };
	m_lodSelection->calculateRanges();

	// Prepare drawing of boxes
	orf_n::DrawPrimitives::getInstance().setupDebugDrawing();

	// Set global shader uniforms valid for all tiles
	m_shaderTerrain->use();
	// Set default global shader uniforms belonging to this quadtree/heightmap
	const float w{ (float)terrain::TILE_SIZE.x };
	const float h{ (float)terrain::TILE_SIZE.y };
	// Used to clamp edges to correct terrain size (only max-es needs clamping, min-s are clamped implicitly)
	m_worldToTexture = omath::vec2{ ( w - 1.0f ) / w, ( h - 1.0f ) / h };
	m_heightMapInfo = omath::vec4{ w, h, 1.0f / w, 1.0f / h };
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_samplerWorldToTextureScale", m_worldToTexture );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_heightmapTextureInfo", m_heightMapInfo );
	// Set dimensions of the gridmesh used for rendering an individual node
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_gridDim", omath::vec3{
		(float)terrain::GRIDMESH_DIMENSION,
		(float)terrain::GRIDMESH_DIMENSION * 0.5f,
		2.0f / (float)terrain::GRIDMESH_DIMENSION
	} );
	// @todo: Global lighting. In the long run, this will be done elsewhere ..
	const omath::vec4 lightColorAmbient{ 0.35f, 0.35f, 0.35f, 1.0f };
	const omath::vec4 lightColorDiffuse{ 0.65f, 0.65f, 0.65f, 1.0f };
	const omath::vec4 fogColor{ 0.0f, 0.5f, 0.5f, 1.0f };
	const omath::vec4 colorMult{ 1.0f, 1.0f, 1.0f, 1.0f };
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_lightColorDiffuse", lightColorDiffuse );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_lightColorAmbient", lightColorAmbient );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_fogColor", fogColor );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_colorMult", colorMult );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_diffuseLightDir", m_diffuseLightPos );

}

void StreamingTerrain::render() {
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );

	// Perform selection @todo parametrize sorting and concatenate lod selection
	// reset selection, add nodes, sort selection, sort by tile index, nearest to farest
	m_lodSelection->reset();
	for( int i{ 0 }; i < terrain::MAX_NUMBER_OF_TILES; ++i ) {
		m_lodSelection->m_currentTileIndex = i;
		m_terrainTiles[i]->getQuadTree()->lodSelect( m_lodSelection );
	}
	m_lodSelection->setDistancesAndSort();

	// Debug: draw bounding boxes
	bool refreshUniforms{ refreshUI() };
	if( m_showTileBoxes || m_showLowestLevelBoxes || m_showSelectedBoxes )
		debugDrawing();

	// Terrain rendering starts here
	if( !m_drawSelection )
		return;
	m_drawGridMesh->bind();
	m_renderStats.reset();
	m_shaderTerrain->use();
	const omath::vec3 observerPos{ m_scene->getCamera()->getPosition() };
	if( refreshUniforms )
		orf_n::setUniform( m_shaderTerrain->getProgram(), "g_diffuseLightDir", m_diffuseLightPos );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "projViewMatrix", m_scene->getCamera()->getViewPerspectiveMatrix() );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "debugColor", orf_n::color::gray );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "g_cameraPos", observerPos );

	GLint drawMode{ m_scene->getCamera()->getWireframeMode() ? GL_LINES : GL_TRIANGLES };

	for( size_t i{0}; i < m_terrainTiles.size(); ++i )
		m_terrainTiles[i]->render( m_shaderTerrain, m_drawGridMesh, m_lodSelection, (int)i, drawMode );

}

void StreamingTerrain::cleanup() {
	delete m_lodSelection;
}

// ******** Debug stuff
void StreamingTerrain::debugDrawing() {
	// To keep the below less verbose
	const orf_n::Program *p{ m_drawPrimitives.getProgramPtr() };
	p->use();
	orf_n::setUniform( p->getProgram(), "projViewMatrix", m_scene->getCamera()->getViewPerspectiveMatrix() );
	glPointSize( 3.0f );
	for( size_t i{0}; i < m_terrainTiles.size(); ++i ) {
		if( m_showTileBoxes )
			m_drawPrimitives.drawABB( *( m_terrainTiles[i]->getTileBoundingBox() ), orf_n::color::white );
		if( m_showLowestLevelBoxes )
			debugDrawLowestLevelBoxes( m_terrainTiles[i] );
		if( m_showSelectedBoxes ) {
			for( int i{ 0 }; i < m_lodSelection->m_selectionCount; ++i ) {
				const terrain::LODSelection::selectedNode_t &n = m_lodSelection->m_selectedNodes[i];
				bool drawFull = n.hasTL && n.hasTR && n.hasBL && n.hasBR;
				if( drawFull ) {
					orf_n::AABB b{ n.node->getBoundingBox()->expand( -0.003f ) };
					m_drawPrimitives.drawABB( b, orf_n::color::rainbow[n.node->getLevel()] );
				} else {
					if( n.hasTL ) {
						orf_n::AABB b{ n.node->getUpperLeft()->getBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawABB( b, orf_n::color::rainbow[n.node->getUpperLeft()->getLevel()] );
					}
					if( n.hasTR ) {
						orf_n::AABB b{ n.node->getUpperRight()->getBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawABB( b, orf_n::color::rainbow[n.node->getUpperRight()->getLevel()] );
					}
					if( n.hasBL ) {
						orf_n::AABB b{ n.node->getLowerLeft()->getBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawABB( b, orf_n::color::rainbow[n.node->getLowerLeft()->getLevel()] );
					}
					if( n.hasBR ) {
						orf_n::AABB b{ n.node->getLowerRight()->getBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawABB( b, orf_n::color::rainbow[n.node->getLowerRight()->getLevel()] );
					}
				}
			}
		}
	}
	glPointSize( 1.0f );
}

// Draw bounding boxes if in frustum
void StreamingTerrain::debugDrawLowestLevelBoxes( const terrain::TerrainTile *const t ) const {
	const terrain::Node *nodes{ t->getQuadTree()->getNodes() };
	for( int i{ 0 }; i < t->getQuadTree()->getNodeCount(); ++i )
		if( nodes[i].getLevel() == terrain::NUMBER_OF_LOD_LEVELS - 1 ) {
			const orf_n::AABB box{ *( nodes[i].getBoundingBox() ) };
			if( m_scene->getCamera()->getViewFrustum().isBoxInFrustum( &box ) != orf_n::OUTSIDE )
				m_drawPrimitives.drawABB( box, orf_n::color::cornflowerBlue );
		}
}

bool StreamingTerrain::refreshUI() {
	bool retVal{ false };
	// UI stuff
	if( orf_n::globals::showAppUI ) {
		ImGui::Begin( "Basic LOD params" );
		ImGui::Text( "Show bounding boxes:" );
		ImGui::Checkbox( "  of tiles map", &m_showTileBoxes );
		ImGui::Checkbox( "  in viewfrustum", &m_showLowestLevelBoxes );
		ImGui::Checkbox( "  LOD selected", &m_showSelectedBoxes );
		ImGui::Checkbox( "Show lod", &m_drawSelection );
		ImGui::Separator();
		ImGui::Text( "Render stats" );
		ImGui::RadioButton( "Too close !", m_lodSelection->m_isVisibilityDistanceTooSmall );
		ImGui::Text( "# selected nodes %d", m_lodSelection->m_selectionCount );
		ImGui::Text( "# rendered nodes %d", m_renderStats.totalRenderedQuads );
		ImGui::Text( "# rendered triangles %d", m_renderStats.totalRenderedTriangles );
		ImGui::Text( "min selected LOD level %d", m_lodSelection->m_minSelectedLODLevel );
		ImGui::Text( "max selected LOD level %d", m_lodSelection->m_maxSelectedLODLevel );
		ImGui::Separator();
		float nearPlane{ m_scene->getCamera()->getNearPlane() };
		float farPlane{ m_scene->getCamera()->getFarPlane() };
		const omath::vec3 oldDiffuseLightPos{ m_diffuseLightPos };
		ImGui::Text( "Camera Control" );
		ImGui::SliderFloat( "Near plane", &nearPlane, 1.0f, 100.0f );
		ImGui::SliderFloat( "Far plane", &farPlane, 200.0f, 2000.0f );
		ImGui::SliderFloat( "Light Position x", &m_diffuseLightPos.x, -10.0f, 10.0f );
		ImGui::End();
		// Recalc camera fov and lod ranges on change
		if( nearPlane != m_scene->getCamera()->getNearPlane() ) {
			m_scene->getCamera()->setNearPlane( nearPlane );
			m_lodSelection->calculateRanges();
		}
		if( farPlane != m_scene->getCamera()->getFarPlane()  ) {
			m_scene->getCamera()->setFarPlane( farPlane );
			m_lodSelection->calculateRanges();
		}
		if( oldDiffuseLightPos != m_diffuseLightPos )
			retVal = true;
	}
	return retVal;
}
