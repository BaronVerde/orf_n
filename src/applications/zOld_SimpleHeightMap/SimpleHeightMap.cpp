
/**
 * @todo: per fragment shading to avoid normal calculation on the cpu
 * compute geodetic surface normals and texture coordinates, p.139
 * grid shading pp. 42ff
 * day/night and terminator blending pp.146ff
 * @todo u_positionToRepeatTextureCoordinate can be used to shade higher resolution textures
 * than the vertex grid is. Avoids blurryness near the viewer.
 * Example: value of 4 -> 4*4 texture coords for each position.
 * @todo Sobel filter sharpness of edges can be adjusted with raising the y(z) value.
 * @todo Use slope (z-component of normal) to clad in grass, sand, rock, and plant things.
 * Slope 0 = 90 degrees, 1 = flat terrain.
 * Use ? for water.
 */

#include <applications/zOld_SimpleHeightMap/SimpleHeightMap.h>
#include <base/Globals.h>
#include <renderer/Sampler.h>
#include <renderer/Uniform.h>
#include <scene/Scene.h>

#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"

using namespace orf_n;

SimpleHeightMap::SimpleHeightMap( const std::string &filename ) :
	Renderable{ "SimpleHeightMap" }, m_filename{ filename } {}

void SimpleHeightMap::setup() {
	// Build flat mesh and read heightmap data
	// @todo: Parametrize bit depht !
	m_heightMap = new terrain::HeightMap( m_filename, terrain::HeightMap::B16, 0 );
	m_flatMesh = new terrain::FlatMesh( m_heightMap->getExtent(), false, 0 );

	// Build shader programs
	std::vector<std::shared_ptr<Module>> modules;
	modules.push_back( std::make_shared<Module>( GL_VERTEX_SHADER,
			"Source/Applications/SimpleHeightMap/SimpleHeightMap.vert.glsl" ) );
	modules.push_back( std::make_shared<Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/SimpleHeightMap/SimpleHeightMap.frag.glsl" ) );
	m_simpleTerrainShader = new Program( modules );
	/*modules.clear();
	modules.push_back( std::make_shared<Module>( GL_VERTEX_SHADER,
			"Source/Applications/SimpleHeightMap/Normals.vert.glsl" ) );
	modules.push_back( std::make_shared<Module>( GL_GEOMETRY_SHADER,
			"Source/Applications/SimpleHeightMap/Normals.geom.glsl" ) );
	modules.push_back( std::make_shared<Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/SimpleHeightMap/Normals.frag.glsl" ) );
	m_normalShader = new Program( modules );*/

	// Load textures
	m_textureColorMap = new Texture2D( "Resources/Textures/Terrain/ColorMap.png", 6 );
	setDefaultSampler( m_textureColorMap->getName(), LINEAR_CLAMP );
	m_textureColorRampHeight = new Texture2D( "Resources/Textures/Terrain/ColorRamp.jpg", 1 );
	setDefaultSampler( m_textureColorRampHeight->getName(), LINEAR_CLAMP );
	m_textureColorRampSlope = new Texture2D( "Resources/Textures/Terrain/ColorRampSlope.jpg", 7 );
	setDefaultSampler( m_textureColorRampSlope->getName(), LINEAR_CLAMP );
	m_textureColorRampGrassStone = new Texture2D( "Resources/Textures/Terrain/BlendRamp.jpg", 2 );	// ?
	setDefaultSampler( m_textureColorRampGrassStone->getName(), LINEAR_CLAMP );
	m_textureGrass = new Texture2D( "Resources/Textures/Terrain/Grass.jpg", 3 );
	setDefaultSampler( m_textureGrass->getName(), LINEAR_REPEAT );
	m_textureStone = new Texture2D( "Resources/Textures/Terrain/Stone.jpg", 4 );
	setDefaultSampler( m_textureStone->getName(), LINEAR_REPEAT );
	m_textureGrassStoneBlendMap = new Texture2D( "Resources/Textures/Terrain/BlendMask.jpg", 5 );	// ?
	setDefaultSampler( m_textureGrassStoneBlendMap->getName(), LINEAR_CLAMP );

	// Set shader vars
	m_simpleTerrainShader->use();
	// Factor in case of terrain::B16
	if( terrain::HeightMap::B16 == m_heightMap->getDepth() )
		m_heightExaggeration *= 100.0f;
	setUniform( m_simpleTerrainShader->getProgram(), "u_heightExaggeration", m_heightExaggeration );
	setUniform( m_simpleTerrainShader->getProgram(), "u_extent", (glm::vec2)m_heightMap->getExtent() );
	setUniform( m_simpleTerrainShader->getProgram(), "u_positionToTextureCoordinate",
			glm::vec2( 1.0 / (float)m_heightMap->getExtent().x, 1.0 / (float)m_heightMap->getExtent().y ) );
	setUniform( m_simpleTerrainShader->getProgram(), "u_positionToRepeatTextureCoordinate",
			glm::vec2( 4.0 / (float)m_heightMap->getExtent().x, 4.0 / (float)m_heightMap->getExtent().y ) );
	setSunPosition( m_sunPosition );
	setSunColor( glm::vec3{ 204.0f, 229.0f, 255.0f } / 255.0f );
	setDiffuseSpecularAmbientShininess( glm::vec4{ 0.65f, 0.25f, 0.1f, 12.0f } );
	setUniform( m_simpleTerrainShader->getProgram(), "u_normalAlgorithm", m_normalAlgorithm );
	setUniform( m_simpleTerrainShader->getProgram(), "u_shadingAlgorithm", m_shadingAlgorithm );
	setUniform( m_simpleTerrainShader->getProgram(), "u_showTerrain", true );
	setUniform( m_simpleTerrainShader->getProgram(), "u_showSilhouette", false );
	float min{ (float)m_heightMap->getMinMaxHeight().x };
	float max{ (float)m_heightMap->getMinMaxHeight().y };
	float minC{ (float)m_heightMap->getMinMaxHeightClamped().x };
	float maxC{ (float)m_heightMap->getMinMaxHeightClamped().y };
	setUniform( m_simpleTerrainShader->getProgram(), "u_minHeight", min );
	setUniform( m_simpleTerrainShader->getProgram(), "u_minHeightClamped", minC );
	setUniform( m_simpleTerrainShader->getProgram(), "u_maxHeight", max );
	setUniform( m_simpleTerrainShader->getProgram(), "u_maxHeightClamped", maxC );
	setUniform( m_simpleTerrainShader->getProgram(), "u_shadingAlgorithm", m_shadingAlgorithm );
	setUniform( m_simpleTerrainShader->getProgram(), "u_normalAlgorithm", m_normalAlgorithm );
	setUniform( m_simpleTerrainShader->getProgram(), "og_highResolutionSnapScale", 1.0f );

	/*m_normalShader->use();
	setUniform( m_normalShader->getProgram(), "u_heightExaggeration", m_heightExaggeration );
	setUniform( m_normalShader->getProgram(), "u_positionToTextureCoordinate",
			glm::vec2( 1.0 / (float)m_heightMap->getExtent().x, 1.0 / (float)m_heightMap->getExtent().y ) );*/

	m_modelMatrix = glm::translate( glm::mat4(),
			glm::vec3( -(float)m_heightMap->getExtent().y / 2.0f,
					   0.0f,
					   -(float)m_heightMap->getExtent().y / 2.0f ) );
	// Initial camera position
	m_scene->getCamera()->setMode( Camera::FIRSTPERSON );
	m_scene->getCamera()->setPosition( { 0.0f, 35.0f, 0.0f } );
	m_scene->getCamera()->setTarget( { 0.0f, 0.0f, 0.0f } );

}

void SimpleHeightMap::render() {
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CW );
	glCullFace( GL_BACK );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

	m_heightMap->bind();
	m_flatMesh->bind();
	if( !m_showNormals )
		m_simpleTerrainShader->use();
	else
		m_normalShader->use();

	// Set shader uniforms like the normal calculation algorithm if they are changed
	int oldNormalAlgorithm{ m_normalAlgorithm };
	int oldShadingAlgorithm{ m_shadingAlgorithm };
	float oldHeightExaggeration{ m_heightExaggeration };
	// bool oldShowNormals{ m_showNormals };
	float oldSobelFactor{ m_sobelFactor };
	glm::vec3 oldSunPosition{ m_sunPosition };
	if( globals::showAppUI ) {
		ImGui::Begin( "Displacement Map Params" );
		ImGui::Text( "Normal algorithm:" );
		ImGui::RadioButton( "None            ", &m_normalAlgorithm, 0 ); ImGui::SameLine();
		ImGui::RadioButton( "Forward diff.", &m_normalAlgorithm, 1 );
		ImGui::RadioButton( "Central diff.   ", &m_normalAlgorithm, 2 ); ImGui::SameLine();
		ImGui::RadioButton( "Sobel filter", &m_normalAlgorithm, 3 );
		ImGui::SliderFloat( "Sobel f", &m_sobelFactor, 0.1f, 20.0f );
		ImGui::Separator();
		ImGui::Text( "Shading algorithm:" );
		ImGui::RadioButton( "Color map       ", &m_shadingAlgorithm, 0 ); ImGui::SameLine();
		ImGui::RadioButton( "Solid", &m_shadingAlgorithm, 1 );
		ImGui::RadioButton( "By height       ", &m_shadingAlgorithm, 2 ); ImGui::SameLine();
		ImGui::RadioButton( "Height contour", &m_shadingAlgorithm, 3 );
		ImGui::RadioButton( "By slope        ", &m_shadingAlgorithm, 6 ); ImGui::SameLine();
		ImGui::RadioButton( "Slope contour", &m_shadingAlgorithm, 7 );
		ImGui::RadioButton( "Slope color ramp", &m_shadingAlgorithm, 8 ); ImGui::SameLine();
		ImGui::RadioButton( "Slope blend ramp", &m_shadingAlgorithm, 9 );
		ImGui::RadioButton( "Blend mask      ", &m_shadingAlgorithm, 10 ); ImGui::SameLine();
		ImGui::RadioButton( "Height color ramp", &m_shadingAlgorithm, 4 );
		ImGui::RadioButton( "Height blend ramp", &m_shadingAlgorithm, 5 );
		ImGui::Checkbox( "Show normals    ", &m_showNormals );
		ImGui::Separator();
		ImGui::SliderFloat( "Height x", &m_heightExaggeration, 0.0f,
				terrain::HeightMap::B16 == m_heightMap->getDepth() ? 1000.0f : 30.0f );
		ImGui::SliderFloat( "Sun position x", &m_sunPosition.x, -500.0f, 500.0f );
		ImGui::SliderFloat( "Sun position y", &m_sunPosition.y, -500.0f, 500.0f );
		ImGui::SliderFloat( "Sun position z", &m_sunPosition.z, -500.0f, 500.0f );
		ImGui::End();
	}
	// Bother OpenGL only in case of change
	if( !m_showNormals ) {
		if( m_normalAlgorithm != oldNormalAlgorithm )
			setUniform( m_simpleTerrainShader->getProgram(), "u_normalAlgorithm", m_normalAlgorithm );
		if( m_shadingAlgorithm != oldShadingAlgorithm )
			setUniform( m_simpleTerrainShader->getProgram(), "u_shadingAlgorithm", m_shadingAlgorithm );
		if( m_heightExaggeration != oldHeightExaggeration )
			setUniform( m_simpleTerrainShader->getProgram(), "u_heightExaggeration", m_heightExaggeration );
		if( m_sunPosition != oldSunPosition )
			setSunPosition( m_sunPosition );
		if( m_sobelFactor != oldSobelFactor )
			setUniform( m_simpleTerrainShader->getProgram(), "u_sobelFactor", m_sobelFactor );
	} else {
		if( m_heightExaggeration != oldHeightExaggeration )
			setUniform( m_normalShader->getProgram(), "u_heightExaggeration", m_heightExaggeration );
		if( m_normalAlgorithm != oldNormalAlgorithm )
			setUniform( m_normalShader->getProgram(), "u_normalAlgorithm", m_normalAlgorithm );
	}

	// Build mvp matrix
	setModelViewProjectionMatrix( m_scene->getCamera()->getPerspectiveProjectionMatrix() *
								  m_scene->getCamera()->getViewMatrix() *
								  m_modelMatrix );
	setCameraPosition( m_scene->getCamera()->getPosition() );

	if( m_scene->getCamera()->getWireframeMode() )
		glDrawElements( GL_LINES, m_flatMesh->getNumIndices(), GL_UNSIGNED_INT, NULL );
	else
		glDrawElements( GL_TRIANGLES, m_flatMesh->getNumIndices(), GL_UNSIGNED_INT, NULL );

}

void SimpleHeightMap::cleanup() {
	delete m_heightMap;
	delete m_flatMesh;
	//delete m_normalShader;
	delete m_simpleTerrainShader;
	delete m_textureColorMap;
	delete m_textureColorRampHeight;
	delete m_textureColorRampSlope;
	delete m_textureColorRampGrassStone;
	delete m_textureGrass;
	delete m_textureStone;
	delete m_textureGrassStoneBlendMap;
}

SimpleHeightMap::~SimpleHeightMap() {}
