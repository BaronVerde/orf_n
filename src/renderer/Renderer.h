
/**
 * Keeps scene(s) to render, creates objects that share data in the scene.
 * Runs the main loop in render().
 * @todo: Resource handling, loading, unloading, keeping track of stuff.
 */

#pragma once

#include <scene/scene.h>
#include "Framebuffer.h"

namespace orf_n {

class Renderer {
public:
	Renderer( bool debug = false );
	virtual ~Renderer();

	/**
	 * Create the window, eventhandler and camera object.
	 * Creates the scene(s) and adds renderable objects.
	 */
	void setupRenderer();

	/**
	 * Call the scene's setup() method to setup all added objects in their order.
	 */
	void setup() const;

	/**
	 * Call the scene's render() method to render objects in their order.
	 */
	void render() const;

	/**
	 * Call the scene's cleanup() method to clean up objects in their order.
	 * @todo: eventually flip cleanup order in respect to setup and render order.
	 */
	void cleanup() const;

	/**
	 * Cleanup renderer specific stuff if necessary.
	 */
	void cleanupRenderer();

private:
	bool m_debug{ false };

	scene *m_scene{ nullptr };

	glfw_window *m_window{ nullptr };

	camera *m_camera{ nullptr };

	Framebuffer *m_framebuffer{ nullptr };

	UIOverlay *m_overlay{ nullptr };

	/**
	 * A stub now. In future, suitability checks should be done here.
	 */
	static bool checkEnvironment();

};

}	// namespace
