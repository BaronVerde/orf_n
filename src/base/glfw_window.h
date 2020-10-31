/**
 * Class to handle a glfw window.
 * @todo resizable and fullsize !
 */

#pragma once

#include "event_handler.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

namespace orf_n {

class event_handler;

class glfw_window {
public:
	/**
     * @brief Creates a new window. Init glfw and glew.
     * @param title Title string for the window
     * @param width Initial window width
     * @param height Initial window heigh
     * @param debug Create debug context and init glfw and opengl callbacks.
     */
    glfw_window( const std::string &title, const int width, const int height, bool debug = false );

    virtual ~glfw_window();

    /**
     * @brief Return glfw window handle.
     */
    GLFWwindow *get_window() const;

    /**
     * @brief Return pointer to event_handler object.
     */
    const event_handler *get_event_handler() const;

    // std::string getTitle() const { return m_title; }

    /**
     * Return framebuffer width and height.
     * @toto Eventually make them float to avoid too many casts.
     */
    int get_width() const;

    void set_width( int width );

    void set_height( int height );

    int get_height() const;

    /**
     * @brief Set the title text of the window.
     */
    void set_title( const std::string &title );

    /**
     * @brief Set should close. Used to end the render loop.
     */
    void set_should_close() const;

    /**
     * @bried Toggle cursor visible and free or captured and hidden.
     */
    void toggle_input_mode();

    bool is_cursor_disabled() const;

    glfw_window &operator=( const glfw_window &eh ) = delete;

    glfw_window( const glfw_window &eh ) = delete;

    bool get_v_sync() const;

    void set_v_sync( bool v );

    void set_damaged( bool damaged );

    bool get_damaged() const;

private:
    GLFWwindow* m_window{nullptr};

    std::unique_ptr<event_handler> m_event_handler{nullptr};

    std::string m_title;

    int m_width;

    int m_height;

    // initial mouse cursor state, see ctor
    bool m_cursorDisabled{ false };

    bool m_isDamaged{ false };

    bool m_debug;

    bool m_vsync;

    static void errorCallback( int error, const char *msg );

    static void APIENTRY glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
    		GLsizei length, const GLchar *message, const void *userParam );

};	// class glfw_window

}	// namespace
