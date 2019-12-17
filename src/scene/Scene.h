
/**
 * Scene holds renderable objects in a map and watches over their life cycle.
 */

#pragma once

#include "applications/Camera/Camera.h"
#include "applications/UIOverlay/UIOverlay.h"
#include "base/GlfwWindow.h"
#include "Renderable.h"

namespace orf_n {

class Scene : public Renderable {
public:
	Scene( GlfwWindow *win, Camera *cam, UIOverlay *ovl );

	virtual ~Scene();

	virtual void setup() override;

	virtual void prepareFrame() override;

	virtual void render() override;

	virtual void endFrame() override;

	virtual void cleanup() override;

	void addRenderable( unsigned int order, std::shared_ptr<Renderable> renderable );

	GlfwWindow *getWindow() const;

	Camera *getCamera() const;

private:
	GlfwWindow *m_window{ nullptr };

	Camera *m_camera{ nullptr };

	UIOverlay *m_overlay{ nullptr };

	/**
	 * Renderables in order for rendering.
	 */
	std::map<unsigned int, std::shared_ptr<Renderable>> m_orderedRenderables;

};

}
