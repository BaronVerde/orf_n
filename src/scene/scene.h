
/**
 * Scene holds renderable objects in a map and watches over their life cycle.
 */

#pragma once

#include <applications/camera/camera.h>
#include <base/glfw_window.h>
#include <scene/renderable.h>
#include "applications/UIOverlay/UIOverlay.h"

namespace orf_n {

class scene : public renderable {
public:
	scene( glfw_window *win, camera *cam, UIOverlay *ovl );

	virtual ~scene();

	virtual void setup() override;

	virtual void prepareFrame() override;

	virtual void render() override;

	virtual void endFrame() override;

	virtual void cleanup() override;

	void add_renderable( unsigned int order, std::shared_ptr<renderable> renderable_object );

	glfw_window *get_window() const;

	camera *get_camera() const;

private:
	glfw_window *m_window{ nullptr };

	camera *m_camera{ nullptr };

	UIOverlay *m_overlay{ nullptr };

	/**
	 * Renderables in order for rendering.
	 */
	std::map<unsigned int, std::shared_ptr<renderable>> m_ordered_renderables;

};

}
