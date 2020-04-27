
#include <base/event_handler.h>
#include <base/glfw_window.h>
#include <base/logbook.h>
#include <iostream>

namespace orf_n {

std::vector<event_handler::registered_object_t> event_handler::s_registered_objects {};

event_handler::event_handler( glfw_window *win ) : m_window { win } {
	glfwSetCursorPosCallback( m_window->get_window(), cursorPosCallback );
	glfwSetKeyCallback( m_window->get_window(), keyCallback );
	glfwSetScrollCallback( m_window->get_window(), scrollCallback );
	glfwSetMouseButtonCallback( m_window->get_window(), mouseButtonCallback );
	glfwSetCharCallback( m_window->get_window(), charCallback );
	glfwSetFramebufferSizeCallback( m_window->get_window(), resizeCallback );
}

event_handler::~event_handler() {}

void event_handler::register_object( registered_object_t &obj ) {
	bool found = false;
	for( registered_object_t &r : s_registered_objects ) {
		if( r.object == obj.object ) {
			logbook::log_msg( logbook::RENDERER, logbook::WARNING,
							 "Ignoring try to register an already registered object with the event handler." );
			found = true;
		}
	}
	if( !found )
		s_registered_objects.push_back( obj );
}

void event_handler::de_register_object( event_handler *eh ) {
	bool found = false;
	for( auto i = s_registered_objects.begin(); i != s_registered_objects.end(); ++i ) {
		if( i->object == eh ) {
			s_registered_objects.erase( i );
			found = true;
			break;
		}
	}
	if( !found )
		logbook::log_msg( logbook::RENDERER, logbook::WARNING,
				"Ignoring try to de-register an unregistered object with the event handler." );
}

// static
void event_handler::keyCallback( GLFWwindow* w, int key, int scancode, int action, int mods ) {
	event_handler *e = reinterpret_cast<event_handler *>( glfwGetWindowUserPointer( w ) );
	e->on_key_pressed( key, scancode, action, mods );
}

// static
void event_handler::cursorPosCallback( GLFWwindow* w, double x, double y ) {
	event_handler *e = reinterpret_cast<event_handler *>( glfwGetWindowUserPointer( w ) );
	e->on_mouse_move( (float)x, (float)y );
}

// static
void event_handler::mouseButtonCallback( GLFWwindow* w, int button, int action, int mods ) {
	event_handler *e = reinterpret_cast<event_handler *>( glfwGetWindowUserPointer( w ) );
	e->on_mouse_button( button, action, mods );
}

// static
void event_handler::scrollCallback( GLFWwindow* w, double xOffset, double yOffset ) {
	event_handler *e = reinterpret_cast<event_handler *>( glfwGetWindowUserPointer( w ) );
	e->on_mouse_scroll( (float)xOffset, (float)yOffset );
}

// static
void event_handler::charCallback( GLFWwindow* w, unsigned int code ) {
	event_handler *e = reinterpret_cast<event_handler *>( glfwGetWindowUserPointer( w ) );
	e->on_char( code );
}

// static
void event_handler::resizeCallback( GLFWwindow* w, int width, int height ) {
	event_handler *e = reinterpret_cast<event_handler *>( glfwGetWindowUserPointer( w ) );
	e->on_framebuffer_resize( width, height );
}

bool event_handler::on_key_pressed( int key, int scancode, int action, int mods ) {
	bool handled = false;
    if( GLFW_PRESS == action || GLFW_REPEAT == action ) {
    	switch( key ) {
    		case GLFW_KEY_ESCAPE:
    			m_window->set_should_close();
    			handled = true;
    			break;
    		case GLFW_KEY_LEFT_ALT:
    			m_window->toggle_input_mode();
    			handled = true;
    			break;
    	}
    }
    // If event has not been handled above, iterate through the registered callbacks
    if( !handled ) {
    	for( registered_object_t &obj : s_registered_objects ) {
    		if( obj.types & KEY ) {
    			//handled = reinterpret_cast<event_handler *>( obj.object )->onKeyPressed(
    			handled = obj.object->on_key_pressed( key, scancode, action, mods );
    			// If callback has been handled, search no more
    			if( handled )
    				break;
    		}
    	}
	}
    return handled;
}	// onKeyPressed()

bool event_handler::on_mouse_move( float x, float y ) {
	bool handled = false;
	for( registered_object_t &obj : s_registered_objects ) {
		if( obj.types & MOUSE_MOVE ) {
			//handled = reinterpret_cast<event_handler *>(obj.object)->onMouseMove( x, y );
			handled = obj.object->on_mouse_move( x, y );
			if( handled )
				break;
		}
	}
	return handled;
}

bool event_handler::on_mouse_button( int button, int action, int mods ) {
	bool handled = true;
	// Camera resets cursor if cursor is disabled !
	for( registered_object_t &obj : s_registered_objects ) {
		if( obj.types & MOUSE_BUTTON ) {
			handled = obj.object->on_mouse_button(
					button, action, mods );
			if( handled )
				break;
		}
	}
	return handled;
}	// onMouseButton()

bool event_handler::on_mouse_scroll( float xOffset, float yOffset ) {
	bool handled = false;
	for( registered_object_t &obj : s_registered_objects ) {
		if( obj.types & MOUSE_SCROLL ) {
			handled = obj.object->on_mouse_scroll(
					xOffset, yOffset );
			if( handled )
				break;
		}
	}
	return handled;
}

bool event_handler::on_char( unsigned int code ) {
	bool handled = false;
	for( registered_object_t &obj : s_registered_objects ) {
		if( obj.types & CHAR ) {
			handled = obj.object->on_char( code );
			if( handled )
				break;
		}
	}
	return handled;
}

bool event_handler::on_framebuffer_resize( int width, int height ) {
	bool handled = false;
	for( registered_object_t &obj : s_registered_objects ) {
		if( obj.types & FRAMEBUFFER_RESIZE ) {
			handled = obj.object->on_framebuffer_resize( width, height );
			if( handled )
				break;
		}
	}
	return handled;
}


const glfw_window *event_handler::get_window() const {
	return m_window;
}

}	// namespace
