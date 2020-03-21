
#include <base/logbook.h>
#include <scene/Renderable.h>

namespace orf_n {

Renderable::Renderable( const std::string &name ) : m_name{ name } {}

Renderable::~Renderable() {}

const std::string &Renderable::getName() const {
	return m_name;
}

void Renderable::setScene( Scene *scene ) {
	m_scene = scene;
}

void Renderable::setup() {}

void Renderable::prepareFrame() {}

void Renderable::render() {}

void Renderable::endFrame() {}

void Renderable::cleanup() {}

}
