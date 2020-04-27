
#include <base/globals.h>
#include <base/logbook.h>
#include <scene/scene.h>

namespace orf_n {

scene::scene( glfw_window *win, camera *cam, UIOverlay *ovl ) :
	renderable{ "Scene" }, m_window{ win }, m_camera{ cam }, m_overlay{ ovl } {
	logbook::log_msg( logbook::SCENE, logbook::INFO, "Scene '" + get_name() + "' created" );
}

void scene::setup() {
	m_overlay->set_scene( this );
	m_overlay->setup();
	for( auto &e : m_ordered_renderables )
		e.second->setup();
}

void scene::prepareFrame() {
	// first prepare the overlay
	m_overlay->prepareFrame();
	for( auto &e : m_ordered_renderables )
		e.second->prepareFrame();
}

void scene::render() {
	for( auto &e : m_ordered_renderables )
		e.second->render();
	// Render overlay last
	m_overlay->render();
}

void scene::endFrame() {
	for( auto &e : m_ordered_renderables )
		e.second->endFrame();
}

void scene::cleanup() {
	for( auto &e : m_ordered_renderables )
		e.second->cleanup();
	m_overlay->cleanup();
}

void scene::add_renderable( unsigned int order, std::shared_ptr<renderable> renderable_object ) {
	renderable_object->set_scene( this );
	auto erg{ m_ordered_renderables.insert( { order, renderable_object } ) };
	if( false != erg.second ) {
		std::string s{ "Object '" + renderable_object->get_name() + "' added to Scene '" + get_name() + "'" };
		logbook::log_msg( logbook::SCENE, logbook::INFO, s );
	} else {
		std::string s{
			"Object '" + renderable_object->get_name() + "' NOT added to Scene '" + get_name() + "'. Duplicate order ?"
		};
		logbook::log_msg( logbook::SCENE, logbook::ERROR, s );
	}
}

glfw_window *scene::get_window() const {
	return m_window;
}

camera *scene::get_camera() const {
	return m_camera;
}

scene::~scene() {
	// m_orderedRenderables.clear();
	logbook::log_msg( logbook::SCENE, logbook::INFO, "Scene '" + get_name() + "' destroyed." );
}

}
