
#include <applications/Camera/Camera.h>
#include <applications/WorldTerrainLOD/GridMesh.h>
#include <applications/WorldTerrainLOD/LODSelection.h>
#include <applications/WorldTerrainLOD/Node.h>
#include <applications/WorldTerrainLOD/QuadTree.h>
#include <applications/WorldTerrainLOD/TerrainLOD.h>
#include <applications/WorldTerrainLOD/TerrainTile.h>
#include <base/Logbook.h>
#include <geometry/AABB.h>
#include <geometry/Ellipsoid.h>
#include <omath/mat4.h>
#include <renderer/IndexBuffer.h>
#include <renderer/Module.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>

TerrainLOD::TerrainLOD() : Renderable( "TerrainLOD" ) {
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
	m_drawGridMesh = std::make_unique<terrain::GridMesh>( terrain::GRIDMESH_DIMENSION );
	// Prepare reference ellispoid
	m_ellipsoid = std::make_unique<orf_n::Ellipsoid>( orf_n::Ellipsoid::WGS84_ELLIPSOID );
	// Create terrain shaders
	std::vector<std::shared_ptr<orf_n::Module>> modules;
	modules.push_back( std::make_shared<orf_n::Module>( GL_VERTEX_SHADER,
			"Source/Applications/WorldTerrainLOD/Terrain.vert.glsl" ) );
	modules.push_back( std::make_shared<orf_n::Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/WorldTerrainLOD/Terrain.frag.glsl" ) );
	m_shaderTerrain = std::make_unique<orf_n::Program>( modules );
}

TerrainLOD::~TerrainLOD() {
	for( size_t i{0}; i < MAX_NUMBER_OF_TILES; ++i )
		delete m_terrainTiles[i];
}

void TerrainLOD::setup() {
	m_terrainTiles.resize( MAX_NUMBER_OF_TILES );
	for( int i{0}; i < MAX_NUMBER_OF_TILES; ++i )
		m_terrainTiles[i] = new terrain::TerrainTile{ TERRAIN_FILES[i], m_ellipsoid.get() };
	// Camera and selection object. Are connected because selection is based on view frustum and range.
	// @todo parametrize or calculate initial position, direction and view range
	m_scene->getCamera()->setPositionAndTarget(
			m_terrainTiles[0]->getAABB()->m_min, m_terrainTiles[0]->getAABB()->m_max
	);
	m_scene->getCamera()->setNearPlane( 100.0f );
	m_scene->getCamera()->setFarPlane( 500000.0f );
	m_scene->getCamera()->setMovementSpeed( 300.0f );
	m_scene->getCamera()->calculateFOV();

	// @todo: no sorting for now. Should be sorted by tileIndex, distanceToCamera and lodLevel
	// to avoid too many heightmap switches and shader uniform settings.
	// Lod selection ranges depend on camera near/far plane distances
	m_lodSelection = new terrain::LODSelection{ m_scene->getCamera(), false /*don't sort*/ };
	m_drawPrimitives.setupDebugDrawing();
	// Set global shader uniforms valid for all tiles
	m_shaderTerrain->use();
	// Set default global shader uniforms belonging to this quadtree/heightmap
	const float w{ static_cast<float>(terrain::TILE_SIZE.x) };
	const float h{ static_cast<float>(terrain::TILE_SIZE.y) };
	// Used to clamp edges to correct terrain size (only max-es needs clamping, min-s are clamped implicitly)
	m_tileToTexture = omath::vec2{ ( w - 1.0f ) / w, ( h - 1.0f ) / h };
	m_heightMapInfo = omath::vec4{ w, h, 1.0f / w, 1.0f / h };
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_tileToTexture", m_tileToTexture );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_heightmapTextureInfo", m_heightMapInfo );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_oneOverRadiiSquared", omath::vec3{m_ellipsoid->getOneOverRadiiSquared()} );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_radiiSquared", omath::vec3{m_ellipsoid->getRadiiSquared()} );
	// Set dimensions of the gridmesh used for rendering an individual node
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_gridDim", omath::vec3{
		static_cast<float>(terrain::GRIDMESH_DIMENSION),
		static_cast<float>(terrain::GRIDMESH_DIMENSION) * 0.5f,
		2.0f / static_cast<float>(terrain::GRIDMESH_DIMENSION)
	} );
	// @todo: Global lighting. In the long run, this will be done elsewhere ...
	const omath::vec4 lightColorAmbient{ 0.35f, 0.35f, 0.35f, 1.0f };
	const omath::vec4 lightColorDiffuse{ 0.65f, 0.65f, 0.65f, 1.0f };
	const omath::vec4 fogColor{ 0.0f, 0.5f, 0.5f, 1.0f };
	const omath::vec4 colorMult{ 1.0f, 1.0f, 1.0f, 1.0f };
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_lightColorDiffuse", lightColorDiffuse );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_lightColorAmbient", lightColorAmbient );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_fogColor", fogColor );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_colorMult", colorMult );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "u_diffuseLightDir", m_diffuseLightPos );
}

void TerrainLOD::render() {
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	const orf_n::Camera *const cam{ m_scene->getCamera() };

	// Perform selection @todo parametrize sorting and concatenate lod selection
	// reset selection, add nodes, sort selection, sort by tile index, nearest to farest
	m_lodSelection->reset();
	for( int i{0}; i < MAX_NUMBER_OF_TILES; ++i ) {
		m_lodSelection->m_currentTileIndex = i;
		m_terrainTiles[i]->getQuadTree()->lodSelect( m_lodSelection );
	}
	m_lodSelection->setDistancesAndSort();

	// Debug: draw bounding boxes
	bool refreshUniforms{ refreshUI() };
	if( m_showTileBoxes || m_showLowestLevelBoxes || m_showSelectedBoxes )
		debugDrawing();

	// Bind meshes, shader, reset stats, prepare and set matrices and cam pos
	if( !m_drawSelection )
		return;
	m_drawGridMesh->bind();
	m_renderStats.reset();
	m_shaderTerrain->use();
	if( refreshUniforms )
		orf_n::setUniform( m_shaderTerrain->getProgram(), "u_diffuseLightDir", m_diffuseLightPos );
	// Build view projection matrix relative to eye
	const omath::dmat4 view{
		omath::lookAt( cam->getPosition(), cam->getPosition() + omath::dvec3{cam->getFront()}, omath::dvec3{cam->getUp()} )
	};
	// Identity matrix as model matrix so far
	const omath::dmat4 mv{ view * omath::dmat4{ 1.0 } };
	// this matrix can be used for all objects in same coord system
	const omath::mat4 mvRTE{
		omath::vec4{mv[0]}, omath::vec4{mv[1]}, omath::vec4{mv[2]}, omath::vec4{ 0.0f, 0.0f, 0.0f, static_cast<float>(mv[3][3]) }
	};
	orf_n::setModelViewProjectionMatrixRTE( cam->getPerspectiveMatrix() * mvRTE );
	orf_n::setViewProjectionMatrix( cam->getViewPerspectiveMatrix() );
	orf_n::setUniform( m_shaderTerrain->getProgram(), "debugColor", orf_n::color::gray );
	orf_n::setCameraPosition( cam->getPosition() );

	// Draw tile by tile
	GLint drawMode{ cam->getWireframeMode() ? GL_LINES : GL_TRIANGLES };
	for( size_t i{0}; i < m_terrainTiles.size(); ++i ) {
		// set tile world coords
		const orf_n::AABB *const aabb{ m_terrainTiles[i]->getAABB() };
		orf_n::setUniform( m_shaderTerrain->getProgram(), "u_tileMax", omath::vec2{ aabb->m_max.x, aabb->m_max.z } );
		orf_n::setUniform( m_shaderTerrain->getProgram(), "u_tileScale", omath::vec3{ aabb->m_max - aabb->m_min } );
		orf_n::setUniform( m_shaderTerrain->getProgram(), "u_tileOffset", omath::vec3{ aabb->m_min } );
		omath::uvec2 renderStats{
			m_terrainTiles[i]->render( m_shaderTerrain.get(), m_drawGridMesh.get(), m_lodSelection, (int)i, drawMode )
		};
		m_renderStats.totalRenderedNodes += renderStats.x;
		m_renderStats.totalRenderedTriangles += renderStats.y;
	}

}

void TerrainLOD::cleanup() {
	delete m_lodSelection;
	m_drawPrimitives.cleanupDebugDrawing();
}

// ******** Debug stuff
void TerrainLOD::debugDrawing() {
	// To keep the below less verbose
	const orf_n::Program *p{ m_drawPrimitives.getProgramPtr() };
	p->use();
	orf_n::setUniform( p->getProgram(), "projViewMatrix", m_scene->getCamera()->getViewPerspectiveMatrix() );
	glPointSize( 3.0f );
	for( size_t i{0}; i < m_terrainTiles.size(); ++i ) {
		if( m_showTileBoxes )
			m_drawPrimitives.drawAABB( *( m_terrainTiles[i]->getAABB() ), orf_n::color::white );
		if( m_showLowestLevelBoxes )
			debugDrawLowestLevelBoxes( m_terrainTiles[i] );
		if( m_showSelectedBoxes ) {
			for( int i{ 0 }; i < m_lodSelection->m_selectionCount; ++i ) {
				const terrain::LODSelection::selectedNode_t &n = m_lodSelection->m_selectedNodes[i];
				bool drawFull = n.hasTL && n.hasTR && n.hasBL && n.hasBR;
				if( drawFull ) {
					orf_n::AABB b{ n.node->getWorldBoundingBox()->expand( -0.003f ) };
					m_drawPrimitives.drawAABB( b, orf_n::color::rainbow[n.node->getLevel()] );
				} else {
					if( n.hasTL ) {
						orf_n::AABB b{ n.node->getUpperLeft()->getWorldBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawAABB( b, orf_n::color::rainbow[n.node->getUpperLeft()->getLevel()] );
					}
					if( n.hasTR ) {
						orf_n::AABB b{ n.node->getUpperRight()->getWorldBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawAABB( b, orf_n::color::rainbow[n.node->getUpperRight()->getLevel()] );
					}
					if( n.hasBL ) {
						orf_n::AABB b{ n.node->getLowerLeft()->getWorldBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawAABB( b, orf_n::color::rainbow[n.node->getLowerLeft()->getLevel()] );
					}
					if( n.hasBR ) {
						orf_n::AABB b{ n.node->getLowerRight()->getWorldBoundingBox()->expand( -0.002f ) };
						m_drawPrimitives.drawAABB( b, orf_n::color::rainbow[n.node->getLowerRight()->getLevel()] );
					}
				}
			}
		}
	}
	glPointSize( 1.0f );
}

// Draw all bounding boxes of heightmap
void TerrainLOD::debugDrawLowestLevelBoxes( const terrain::TerrainTile *const t ) const {
	const terrain::Node *const nodes{ t->getQuadTree()->getNodes() };
	for( int i{ 0 }; i < t->getQuadTree()->getNodeCount(); ++i )
		if( nodes[i].getLevel() == terrain::NUMBER_OF_LOD_LEVELS - 1 ) {
			const orf_n::AABB *box{ nodes[i].getWorldBoundingBox() };
			if( m_scene->getCamera()->getViewFrustum().isBoxInFrustum( box ) != orf_n::OUTSIDE )
				m_drawPrimitives.drawAABB( *box, orf_n::color::cornflowerBlue );
		}
}

bool TerrainLOD::refreshUI() {
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
		ImGui::Text( "# selected nodes %d", m_lodSelection->m_selectionCount );
		ImGui::Text( "# rendered nodes %d", m_renderStats.totalRenderedNodes );
		ImGui::Text( "# rendered triangles %d", m_renderStats.totalRenderedTriangles );
		ImGui::Text( "min selected LOD level %d", m_lodSelection->m_minSelectedLODLevel );
		ImGui::Text( "max selected LOD level %d", m_lodSelection->m_maxSelectedLODLevel );
		ImGui::Separator();
		float nearPlane{ m_scene->getCamera()->getNearPlane() };
		float farPlane{ m_scene->getCamera()->getFarPlane() };
		const omath::vec3 oldDiffuseLightPos{ m_diffuseLightPos };
		ImGui::Text( "Camera Control" );
		ImGui::SliderFloat( "Near plane", &nearPlane, 10.0f, 1000.0f );
		ImGui::SliderFloat( "Far plane", &farPlane, 20000.0f, 1000000.0f );
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
