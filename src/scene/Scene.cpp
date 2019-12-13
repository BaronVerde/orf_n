
#include <base/Globals.h>
#include <base/Logbook.h>
#include <scene/Scene.h>

namespace orf_n {

Scene::Scene( GlfwWindow *win, Camera *cam, UIOverlay *ovl ) :
	Renderable{ "Scene" }, m_window{ win }, m_camera{ cam }, m_overlay{ ovl } {
	Logbook::getInstance().logMsg( Logbook::SCENE, Logbook::INFO, "Scene '" + getName() + "' created" );
}

void Scene::setup() {
	m_overlay->setScene( this );
	m_overlay->setup();
	for( auto &e : m_orderedRenderables )
		e.second->setup();
}

void Scene::prepareFrame() {
	// first prepare the overlay
	m_overlay->prepareFrame();
	for( auto &e : m_orderedRenderables )
		e.second->prepareFrame();
}

void Scene::render() {
	for( auto &e : m_orderedRenderables )
		e.second->render();
	// Render overlay last
	m_overlay->render();
}

void Scene::endFrame() {
	for( auto &e : m_orderedRenderables )
		e.second->endFrame();
}

void Scene::cleanup() {
	for( auto &e : m_orderedRenderables )
		e.second->cleanup();
	m_overlay->cleanup();
}

void Scene::addRenderable( unsigned int order, std::shared_ptr<Renderable> renderable ) {
	renderable->setScene( this );
	auto erg{ m_orderedRenderables.insert( { order, renderable } ) };
	if( false != erg.second ) {
		std::string s{ "Object '" + renderable->getName() + "' added to Scene '" + getName() + "'" };
		Logbook::getInstance().logMsg( Logbook::SCENE, Logbook::INFO, s );
	} else {
		std::string s{ "Object '" + renderable->getName() + "' NOT added to Scene '" + getName() + "'. Duplicate order ?" };
		Logbook::getInstance().logMsg( Logbook::SCENE, Logbook::ERROR, s );
	}
}

GlfwWindow *Scene::getWindow() const {
	return m_window;
}

Camera *Scene::getCamera() const {
	return m_camera;
}

Scene::~Scene() {
	// m_orderedRenderables.clear();
	Logbook::getInstance().logMsg( Logbook::SCENE, Logbook::INFO, "Scene '" + getName() + "' destroyed." );
}

}
