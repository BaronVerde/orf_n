
#include <base/EventHandler.h>
#include <base/Logbook.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace orf_n {

std::vector<EventHandler::RegisteredObject> EventHandler::s_registeredObjects {};

EventHandler::EventHandler( GlfwWindow *win ) : m_window { win } {
	glfwSetCursorPosCallback( m_window->getWindow(), cursorPosCallback );
	glfwSetKeyCallback( m_window->getWindow(), keyCallback );
	glfwSetScrollCallback( m_window->getWindow(), scrollCallback );
	glfwSetMouseButtonCallback( m_window->getWindow(), mouseButtonCallback );
	glfwSetCharCallback( m_window->getWindow(), charCallback );
	glfwSetFramebufferSizeCallback( m_window->getWindow(), resizeCallback );
}

EventHandler::~EventHandler() {}

void EventHandler::registerObject( RegisteredObject &obj ) {
	bool found = false;
	for( RegisteredObject &r : s_registeredObjects ) {
		if( r.object == obj.object ) {
			Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::WARNING,
							 "Ignoring try to register an already registered object with the event handler." );
			found = true;
		}
	}
	if( !found )
		s_registeredObjects.push_back( obj );
}

void EventHandler::deRegisterObject( EventHandler *eh ) {
	bool found = false;
	for( auto i = s_registeredObjects.begin(); i != s_registeredObjects.end(); ++i ) {
		if( i->object == eh ) {
			s_registeredObjects.erase( i );
			found = true;
			break;
		}
	}
	if( !found )
		Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::WARNING,
				"Ignoring try to de-register an unregistered object with the event handler." );
}

// static
void EventHandler::keyCallback( GLFWwindow *w, int key, int scancode, int action, int mods ) {
	EventHandler *e = reinterpret_cast<EventHandler *>( glfwGetWindowUserPointer( w ) );
	e->onKeyPressed( key, scancode, action, mods );
}

// static
void EventHandler::cursorPosCallback( GLFWwindow *w, double x, double y ) {
	EventHandler *e = reinterpret_cast<EventHandler *>( glfwGetWindowUserPointer( w ) );
	e->onMouseMove( (float)x, (float)y );
}

// static
void EventHandler::mouseButtonCallback( GLFWwindow *w, int button, int action, int mods ) {
	EventHandler *e = reinterpret_cast<EventHandler *>( glfwGetWindowUserPointer( w ) );
	e->onMouseButton( button, action, mods );
}

// static
void EventHandler::scrollCallback( GLFWwindow *w, double xOffset, double yOffset ) {
	EventHandler *e = reinterpret_cast<EventHandler *>( glfwGetWindowUserPointer( w ) );
	e->onMouseScroll( (float)xOffset, (float)yOffset );
}

// static
void EventHandler::charCallback( GLFWwindow *w, unsigned int code ) {
	EventHandler *e = reinterpret_cast<EventHandler *>( glfwGetWindowUserPointer( w ) );
	e->onChar( code );
}

// static
void EventHandler::resizeCallback( GLFWwindow *w, int width, int height ) {
	EventHandler *e = reinterpret_cast<EventHandler *>( glfwGetWindowUserPointer( w ) );
	e->onFramebufferResize( width, height );
}

bool EventHandler::onKeyPressed( int key, int scancode, int action, int mods ) {
	bool handled = false;
    if( GLFW_PRESS == action || GLFW_REPEAT == action ) {
    	switch( key ) {
    		case GLFW_KEY_ESCAPE:
    			m_window->setShouldClose();
    			handled = true;
    			break;
    		case GLFW_KEY_LEFT_ALT:
    			m_window->toggleInputMode();
    			handled = true;
    			break;
    	}
    }
    // If event has not been handled above, iterate through the registered callbacks
    if( !handled ) {
    	for( RegisteredObject &obj : s_registeredObjects ) {
    		if( obj.types & KEY ) {
    			//handled = reinterpret_cast<EventHandler *>( obj.object )->onKeyPressed(
    			handled = obj.object->onKeyPressed( key, scancode, action, mods );
    			// If callback has been handled, search no more
    			if( handled )
    				break;
    		}
    	}
	}
    return handled;
}	// onKeyPressed()

bool EventHandler::onMouseMove( float x, float y ) {
	bool handled = false;
	for( RegisteredObject &obj : s_registeredObjects ) {
		if( obj.types & MOUSE_MOVE ) {
			//handled = reinterpret_cast<EventHandler *>(obj.object)->onMouseMove( x, y );
			handled = obj.object->onMouseMove( x, y );
			if( handled )
				break;
		}
	}
	return handled;
}

bool EventHandler::onMouseButton( int button, int action, int mods ) {
	bool handled = true;
	// Camera resets cursor if cursor is disabled !
	for( RegisteredObject &obj : s_registeredObjects ) {
		if( obj.types & MOUSE_BUTTON ) {
			handled = obj.object->onMouseButton(
					button, action, mods );
			if( handled )
				break;
		}
	}
	return handled;
}	// onMouseButton()

bool EventHandler::onMouseScroll( float xOffset, float yOffset ) {
	bool handled = false;
	for( RegisteredObject &obj : s_registeredObjects ) {
		if( obj.types & MOUSE_SCROLL ) {
			handled = obj.object->onMouseScroll(
					xOffset, yOffset );
			if( handled )
				break;
		}
	}
	return handled;
}

bool EventHandler::onChar( unsigned int code ) {
	bool handled = false;
	for( RegisteredObject &obj : s_registeredObjects ) {
		if( obj.types & CHAR ) {
			handled = obj.object->onChar( code );
			if( handled )
				break;
		}
	}
	return handled;
}

bool EventHandler::onFramebufferResize( int width, int height ) {
	bool handled = false;
	for( RegisteredObject &obj : s_registeredObjects ) {
		if( obj.types & FRAMEBUFFER_RESIZE ) {
			handled = obj.object->onFramebufferResize( width, height );
			if( handled )
				break;
		}
	}
	return handled;
}


const GlfwWindow *EventHandler::getWindow() const {
	return m_window;
}

}	// namespace
