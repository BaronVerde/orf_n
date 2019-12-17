/**
 * Handles input events from a GlfwWindow instance
 */

#pragma once

#include <vector>

struct GLFWwindow;

namespace orf_n {

class GlfwWindow;

class EventHandler {
public:
	typedef enum eventType : unsigned int {
		MOUSE_MOVE = 0b1,
		MOUSE_BUTTON = 0b10,
		MOUSE_SCROLL = 0b100,
		KEY = 0b1000,
		CHAR = 0b10000,
		FRAMEBUFFER_RESIZE = 0b100000
	} eventType;

	struct RegisteredObject {
		EventHandler *object;
		unsigned int types;
	};

    EventHandler( GlfwWindow *win );

    virtual ~EventHandler();

    void registerObject( RegisteredObject &obj );

    void deRegisterObject( EventHandler *eh );

    const GlfwWindow *getWindow() const;

    EventHandler &operator=( const EventHandler &eh ) = delete;

    EventHandler( const EventHandler &eh ) = delete;

    static std::vector<RegisteredObject> s_registeredObjects;

	/**
	 * @brief To be overridden by inheriting objects.
	 */
    virtual bool onMouseMove( float x, float y );

    virtual bool onMouseButton( int button, int action, int mods );

    virtual bool onMouseScroll( float xOffset, float yOffset );

    virtual bool onKeyPressed( int key, int scancode, int action, int mods );

    virtual bool onChar( unsigned int code );

    virtual bool onFramebufferResize( int width, int height );

private:
    GlfwWindow *m_window{nullptr};

    static void keyCallback( GLFWwindow *window, int key, int scancode, int action, int mods );

    static void charCallback( GLFWwindow *window, unsigned int code );

    static void cursorPosCallback( GLFWwindow *window, double xPos, double yPos );

    static void mouseButtonCallback( GLFWwindow *win, int button, int action, int mods );

    static void scrollCallback( GLFWwindow *win, double xoffset, double yoffset );

    static void resizeCallback( GLFWwindow *win, int width, int height );

};

}	// namespace
