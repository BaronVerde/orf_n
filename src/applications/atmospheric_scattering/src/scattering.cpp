
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "atmosphere/atmosphere.h"

int window_width_ = 1280;
int window_height_ = 800;
bool cursor_hidden = false;
GLFWwindow* window_ = nullptr;
scattering::atmosphere* atm;

void window_size_callback( GLFWwindow* w, int viewport_width, int viewport_height );
void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods );
void cursor_position_callback( GLFWwindow* window, double xpos, double ypos );
void mouse_button_callback( GLFWwindow* window, int button, int action, int mods );
void scroll_callback( GLFWwindow* window, double xoffset, double yoffset );

void error_callback( int error, const char *msg );
void APIENTRY glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *message, const void *userParam );
void cleanup_graphics();
void init_graphics();

int main() {
	puts( "Startup ..." );
	init_graphics();
	atm = new scattering::atmosphere();

	atm->setup( window_width_, window_height_ );

	while( !glfwWindowShouldClose( window_ ) ) {
		atm->render();
		glfwPollEvents();
		glfwSwapBuffers( window_ );
	}

	atm->cleanup();

	delete atm;
	cleanup_graphics();
	puts( "... ending" );
	return EXIT_SUCCESS;
}

void error_callback( int error, const char *msg ) {
	std::string s;
	s = " [" + std::to_string(error) + "] " + msg + '\n';
	fputs( s.c_str(), stderr );
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void APIENTRY glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *message, const void *userParam ) {
	// Ignore non-significant buffer usage
	if( id == 131185 )
		return;
    std::string s = "Debug message (" + std::to_string(id) + "): ";
    switch( source ) {
        case GL_DEBUG_SOURCE_API:             s += "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   s += "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: s += "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     s += "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     s += "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           s += "Source: Other"; break;
    }
    s += "; ";
    switch( type ) {
        case GL_DEBUG_TYPE_ERROR:               s += "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: s += "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  s += "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         s += "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         s += "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              s += "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          s += "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           s += "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               s += "Type: Other"; break;
    }
    s += "; ";
    switch( severity ) {
        case GL_DEBUG_SEVERITY_HIGH:         s += "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       s += "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          s += "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: s += "Severity: notification"; break;
    }
    s += "; ";
    s += message;
    fputs( s.c_str(), stderr );
    fputc( '\n', stderr );
}

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ) {
	bool handled{ false };
	if( GLFW_PRESS == action ) {
		switch( key ) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose( window, GLFW_TRUE );
				handled = true;
				break;
			case GLFW_KEY_LEFT_ALT:
				cursor_hidden = !cursor_hidden;
				if( cursor_hidden )
					glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
				else
					glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
				handled = true;
				break;
		}
	}
	if( !handled )
		atm->key_event( key, action, mods );
}

void mouse_button_callback( GLFWwindow* window, int button, int action, int mods ) {
	atm->mouse_button_event( window, button, action, mods );
}

void cursor_position_callback( GLFWwindow* window, double mouse_x, double mouse_y ) {
	atm->mouse_move_event( mouse_x, mouse_y );
}

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset ) {
	atm->scroll_event( xoffset, yoffset );
}
#pragma GCC diagnostic pop

void cleanup_graphics() {
	 glfwDestroyWindow( window_ );
	 glfwTerminate();
}

void init_graphics() {
	glfwSetErrorCallback( error_callback );
	if( GL_TRUE != glfwInit() )
		fputs( "Error initialising glfw\n", stderr );
	// Window and context; we want OpenGL 4.5
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
	glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	glfwWindowHint( GLFW_SAMPLES, 0 );
	glfwWindowHint( GLFW_DEPTH_BITS, 32 );
	window_ = glfwCreateWindow( window_width_, window_height_, "Scattering", nullptr, nullptr );
	if( NULL == window_ )
		fputs( "Could not create glfw window\n", stderr );
	glfwMakeContextCurrent( window_ );
	if( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) )
		fputs( "Error loading glad\n", stderr );
	puts( "OpenGL 4.5 context created." );
	glfwSwapInterval( 1 );
	glfwSetInputMode( window_, GLFW_STICKY_KEYS, GLFW_TRUE );
	GLint flags;
	glGetIntegerv( GL_CONTEXT_FLAGS, &flags );
	if( flags & GL_CONTEXT_FLAG_DEBUG_BIT ) {
		glEnable( GL_DEBUG_OUTPUT );
		// Envoke callback directly in case of error
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
		glDebugMessageCallback( glDebugOutput, nullptr );
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
		puts( "Debug context created." );
	} else
		puts( "Debug context not created. Continuing without debug messages." );
	glfwSetKeyCallback( window_, key_callback );
	glfwSetCursorPosCallback( window_, cursor_position_callback );
	glfwSetMouseButtonCallback( window_, mouse_button_callback );
	glfwSetScrollCallback( window_, scroll_callback );
}
