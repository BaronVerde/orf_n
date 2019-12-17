/**
 * Class to handle a glfw window.
 * @todo resizable and fullsize !
 */

#pragma once

#include "EventHandler.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>

namespace orf_n {

class EventHandler;

class GlfwWindow {
public:
	/**
     * @brief Creates a new window. Init glfw and glew.
     * @param title Title string for the window
     * @param width Initial window width
     * @param height Initial window heigh
     * @param debug Create debug context and init glfw and opengl callbacks.
     */
    GlfwWindow( const std::string &title, const int width, const int height, bool debug = false );

    virtual ~GlfwWindow();

    /**
     * @brief Return glfw window handle.
     */
    GLFWwindow *getWindow() const;

    /**
     * @brief Return pointer to EventHandler object.
     */
    const EventHandler *getEventHandler() const;

    // std::string getTitle() const { return m_title; }

    /**
     * Return framebuffer width and height.
     * @toto Eventually make them float to avoid too many casts.
     */
    int getWidth() const;

    void setWidth( int width );

    void setHeight( int height );

    int getHeight() const;

    /**
     * @brief Set the title text of the window.
     */
    void setTitle( const std::string &title );

    /**
     * @brief Set should close. Used to end the render loop.
     */
    void setShouldClose() const;

    /**
     * @bried Toggle cursor visible and free or captured and hidden.
     */
    void toggleInputMode();

    bool isCursorDisabled() const;

    GlfwWindow &operator=( const GlfwWindow &eh ) = delete;

    GlfwWindow( const GlfwWindow &eh ) = delete;

    bool getVsync() const;

    void setVsync( bool v );

    void setDamaged( bool damaged );

    bool isDamaged() const;

private:
    GLFWwindow *m_window{nullptr};

    std::unique_ptr<EventHandler> m_eventHandler{nullptr};

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

};	// class GlfwWindow

}	// namespace
