/**
 * Handles input events from a glfw_window instance
 */

#pragma once

#include <vector>

struct GLFWwindow;

namespace orf_n {

class glfw_window;

class event_handler {
public:
	typedef enum event_type : unsigned int {
		MOUSE_MOVE = 0b1,
		MOUSE_BUTTON = 0b10,
		MOUSE_SCROLL = 0b100,
		KEY = 0b1000,
		CHAR = 0b10000,
		FRAMEBUFFER_RESIZE = 0b100000
	} event_type_t;

	typedef struct {
		event_handler *object;
		unsigned int types;
	} registered_object_t;

    event_handler( glfw_window *win );

    virtual ~event_handler();

    void register_object( registered_object_t &obj );

    void de_register_object( event_handler *eh );

    const glfw_window *get_window() const;

    event_handler &operator=( const event_handler &eh ) = delete;

    event_handler( const event_handler &eh ) = delete;

    static std::vector<registered_object_t> s_registered_objects;

	/**
	 * @brief To be overridden by inheriting objects.
	 */
    virtual bool on_mouse_move( float x, float y );

    virtual bool on_mouse_button( int button, int action, int mods );

    virtual bool on_mouse_scroll( float xOffset, float yOffset );

    virtual bool on_key_pressed( int key, int scancode, int action, int mods );

    virtual bool on_char( unsigned int code );

    virtual bool on_framebuffer_resize( int width, int height );

private:
    glfw_window *m_window{nullptr};

    static void keyCallback( GLFWwindow *window, int key, int scancode, int action, int mods );

    static void charCallback( GLFWwindow *window, unsigned int code );

    static void cursorPosCallback( GLFWwindow *window, double xPos, double yPos );

    static void mouseButtonCallback( GLFWwindow *win, int button, int action, int mods );

    static void scrollCallback( GLFWwindow *win, double xoffset, double yoffset );

    static void resizeCallback( GLFWwindow *win, int width, int height );

};

}	// namespace
