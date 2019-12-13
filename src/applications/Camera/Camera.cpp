
#include <applications/Camera/Camera.h>
#include <base/GlfwWindow.h>
#include <base/Globals.h>		// deltaTime
#include <geometry/Plane.h>
#include <geometry/ViewFrustum.h>
#include <omath/mat4.h>
#include <iostream>
#include <string>

#include "../../../extern/imgui/imgui.h"

namespace orf_n {

Camera::Camera( GlfwWindow *win,
				omath::dvec3 position,
				omath::dvec3 target,
				omath::vec3 up,
				float nearPlane,
				float farPlane,
				cameraMode mode ) :
						EventHandler{ win },
						m_window{ win },
						m_position{ position },
						m_target{ target },
						m_nearPlane{ nearPlane },
						m_farPlane{ farPlane },
						m_up{ up },
						m_mode{ mode } {

	// center of the screen for reset of captured mouse coursor.
	m_centerOfScreen = { (float)m_window->getWidth() / 2.0f, (float)m_window->getHeight() / 2.0f };

	// register input events with the event handler
	unsigned int cb = EventHandler::KEY |
					  EventHandler::MOUSE_MOVE |
					  EventHandler::MOUSE_BUTTON |
					  EventHandler::MOUSE_SCROLL |
					  EventHandler::FRAMEBUFFER_RESIZE;
	EventHandler::RegisteredObject o{ this, cb };
	registerObject( o );

	calculateInitialAngles();
	calculateFOV();
	updateCameraVectors();
}

void Camera::calculateFOV() {
	m_perspectiveMatrix = omath::perspective(
			omath::radians( m_zoom ),
			(float)m_window->getWidth() / (float)m_window->getHeight(),
			m_nearPlane, m_farPlane
	);
	/*const float zModMul{ 1.001f };
	const float zModAdd{ 0.01f };
	m_zOffsetPerspectiveMatrix = omath::perspective( omath::radians( m_zoom ),
			(float)m_window->getWidth() / (float)m_window->getHeight(),
			m_nearPlane * zModMul + zModAdd, m_farPlane * zModMul + zModAdd );*/
	m_frustum.setFOV( m_zoom, (float)m_window->getWidth() / (float)m_window->getHeight(),
			m_nearPlane, m_farPlane );
}

void Camera::calculateInitialAngles() {
	// Calculate initial yaw and pitch on change of position or target
	// m_distanceToTarget is float, direction does not need the precision
	// @todo different cases for camera mode
	omath::vec3 direction{ m_target - m_position };
	m_distanceToTarget = omath::magnitude( m_target - m_position );
	std::cout << "Camera: calculating new angles for position " << m_position << "; target " <<
				m_target << "; distance " << m_distanceToTarget << ".\n";
	m_yaw = omath::degrees( std::atan2( -direction.x, direction.z ) );
	m_pitch = omath::degrees( std::atan2( direction.y,
			std::sqrt( ( direction.x * direction.x) + ( direction.y * direction.y ) ) ) );
	updateCameraVectors();
}

void Camera::updateCameraVectors() {
	if( ORBITING == m_mode ) {
		//m_distanceToTarget = omath::length( m_target - m_position );
		const float yaw{ -omath::radians( m_yaw ) };
		const float pitch{ -omath::radians( m_pitch ) };
		const float cp{ std::cos( pitch ) };
		// Calculate the camera position on the sphere around the target
		m_position = omath::dvec3{ m_distanceToTarget * std::sin( yaw ) * cp,
								   m_distanceToTarget * std::sin( pitch ),
								   -m_distanceToTarget * std::cos( yaw ) * cp };
		m_front = omath::normalize( m_target - m_position );
		// conversion to float because precision isn't needed here
		m_viewMatrix = omath::lookAt( omath::vec3{ m_position }, omath::vec3{ m_target }, m_up );
		m_frustum.setCameraVectors( omath::vec3{ m_position }, omath::vec3{ m_target }, m_up );
	} else {
		const omath::vec3 posf{ m_position };
		// flip yaw and pitch because intuition (at least mine ;-))
		const float yaw{ omath::radians( m_pitch ) };
		const float pitch{ omath::radians( m_yaw ) };
		const float cy{ std::cos( yaw ) };
		// Calculate the new front and right vectors. Camera position is set by movement.
		omath::vec3 front{ -cy * std::sin( pitch ),
						  std::sin( yaw ),
						  cy * std::cos( pitch ) };
		m_front = omath::normalize( front );
		m_viewMatrix = omath::lookAt( posf, posf + front, m_up );
		m_frustum.setCameraVectors( posf, posf + front, m_up );
	}
	// Also re-calculate the right vector
	m_right = omath::normalize( omath::cross( m_front, m_up ) );
	// prefab projection * view
	m_viewPerspectiveMatrix = m_perspectiveMatrix * m_viewMatrix;
	// prefab proj * untranslated view matrix, stripped of translation
	m_untranslatedViewPerspectiveMatrix = m_perspectiveMatrix *
			omath::mat4{ m_viewMatrix[0], m_viewMatrix[1], m_viewMatrix[2], omath::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } };
}

// virtual
bool Camera::onMouseMove( float x, float y ) {
	if( !m_window->isCursorDisabled() )
		return false;
	bool handled{ false };
	// @todo/fixme cursor should center automatically, according to glfw doc, but doesn't
	glfwSetCursorPos( m_window->getWindow(), m_centerOfScreen.x, m_centerOfScreen.y );
	m_pitch += m_mouseSensitivity * ( m_centerOfScreen.y - y );
	if( FIRST_PERSON == m_mode )
		m_yaw -= m_mouseSensitivity * ( m_centerOfScreen.x - x );
	else
		m_yaw += m_mouseSensitivity * ( m_centerOfScreen.x - x );
	// So screen doesn't get flipped
	if( m_pitch > 89.0f )
		m_pitch = 89.0f;
	if( m_pitch < -89.0f )
		m_pitch = -89.0f;
	// Update camera vectors and view frustum
	updateCameraVectors();
	handled = true;
	return handled;
}	// onMouseMove()

// virtual
bool Camera::onMouseScroll( float xOffset, float yOffset ) {
	bool handled{ false };
	/* if( yOffset != 0 ) {
		if( m_zoom >= 1.0f && m_zoom <= 60.0f )
			m_zoom -= yOffset;
		if( m_zoom <= 1.0f )
			m_zoom = 1.0f;
		if( m_zoom >= 60.0f )
			m_zoom = 60.0f;
		calculateFOV();
		updateCameraVectors();
		handled = true;
	} */
	return handled;
}	// onMouseScroll()

// virtual
bool Camera::onKeyPressed( int key, int scancode, int action, int mods ) {
	bool handled{ false };
	// movement is mirrored between fps and orbiting mode
    if( GLFW_PRESS == action ) {
    	switch( key ) {
    		case GLFW_KEY_P:
    			printPosition();
    			handled = true;
    			break;
    		case GLFW_KEY_W:
    			m_isMoving = true;
    			if( FIRST_PERSON == m_mode ) {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_FORWARD;
    				else
    					m_direction = FORWARD;
    			} else {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_CLOSE;
    				else
    					m_direction = CLOSE;
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_S:
    			m_isMoving = true;
    			if( FIRST_PERSON == m_mode ) {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_BACKWARD;
    				else
    					m_direction = BACKWARD;
    			} else {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_RETREAT;
    				else
    					m_direction = RETREAT;
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_A:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_LEFT;
    			else
    				m_direction = LEFT;
    			handled = true;
    			break;
    		case GLFW_KEY_D:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_RIGHT;
    			else
    				m_direction = RIGHT;
    			handled = true;
    			break;
    		case GLFW_KEY_Z:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_DOWN;
    			else
    				m_direction = DOWN;
    			handled = true;
    			break;
    		case GLFW_KEY_X:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_UP;
    			else
    				m_direction = UP;
    			handled = true;
    			break;
    		case GLFW_KEY_Q:
    			m_isMoving = true;
    			m_direction = ROTATE_LEFT;
    			break;
    		case GLFW_KEY_E:
    			m_isMoving = true;
    			m_direction = ROTATE_RIGHT;
    			handled = true;
    			break;
    		case GLFW_KEY_TAB:
    			if( ORBITING == m_mode ) {
    				// @todo Set initial values from orbiting cam
    				m_mode = FIRST_PERSON;
    				setPositionAndTarget( m_position, m_target );
    			} else {
    				m_mode = ORBITING;
    				updateCameraVectors();
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_LEFT_ALT:
    			m_window->toggleInputMode();
    			handled = true;
    			break;
    		case GLFW_KEY_F:
    			m_wireframe = !m_wireframe;
    			handled = true;
    			break;
    		case GLFW_KEY_ESCAPE:
    			m_window->setShouldClose();
    			handled = true;
    			break;
    	}
    }
    if( GLFW_RELEASE == action && ( GLFW_KEY_W == key || GLFW_KEY_S == key ||
    								GLFW_KEY_A == key || GLFW_KEY_D == key ||
									GLFW_KEY_Z == key || GLFW_KEY_X == key ||
									GLFW_KEY_Q == key || GLFW_KEY_E == key ) ) {
    	m_isMoving = false;
    	handled = true;
    }
    // If moving, camera vectors and frustum planes are updated in updateMoving()
    return handled;
}	// onKeyPressed()

// virtual
bool Camera::onMouseButton( int button, int action, int mods ) {
	bool handled = false;
	// when clicked, cast ray and find out where
	return handled;
}	// onMouseButton()

bool Camera::onFramebufferResize( int width, int height ) {
	bool handled = false;
	m_window->setWidth( width );
	m_window->setHeight( height );
	calculateFOV();
	// Tell the renderer that framebuffer must be resized
	m_window->setDamaged( true );
	return handled;
}	// onMouseButto

void Camera::updateMoving() {
	/* Has been transferred to the application for view dependent calculations
	if( globals::showAppUI ) {
		const float oldNearPlane{ m_nearPlane };
		const float oldFarPlane{ m_farPlane };
		const float oldZoom{ m_zoom };
		ImGui::Begin( "Camera Control" );
		ImGui::SliderFloat( "Near plane", &m_nearPlane, 0.1f, 10.0f );
		ImGui::SliderFloat( "Far plane", &m_farPlane, 20.0f, 2000.0f );
		ImGui::SliderFloat( "Zoom angle", &m_zoom, 30.0f, 60.0f );
		ImGui::End();
		if( oldNearPlane != m_nearPlane || oldFarPlane != m_farPlane || oldZoom != m_zoom )
			calculateFOV();
	}*/
	if( m_isMoving ) {
		float velocity = m_movementSpeed * (float)globals::deltaTime;
		switch( m_direction ) {
			// up, front and right must be unit vectors or else you will be lost in space.
			case FAST_FORWARD:
				m_position += m_front * 10.0f * velocity;
				break;
			case FORWARD:
				m_position += m_front * velocity;
				break;
			case FAST_BACKWARD:
				m_position -= m_front * 10.0f * velocity;
				break;
			case BACKWARD:
				m_position -= m_front * velocity;
				break;
			case FAST_LEFT:
				m_position -= m_right * 10.0f * velocity;
				break;
			case LEFT:
				m_position -= m_right * velocity;
				break;
			case FAST_RIGHT:
				m_position += m_right * 10.0f * velocity;
				break;
			case RIGHT:
				m_position += m_right * velocity;
				break;
			case FAST_UP:
				m_position += m_up * 10.0f * velocity;
				break;
			case UP:
				m_position += m_up * velocity;
				break;
			case FAST_DOWN:
				m_position -= m_up * 10.0f * velocity;
				break;
			case DOWN:
				m_position -= m_up * velocity;
				break;
			case ROTATE_LEFT:
				break;
			case ROTATE_RIGHT:
				break;
			case FAST_CLOSE:
				m_distanceToTarget -= 10.0f * velocity;
				break;
			case CLOSE:
				m_distanceToTarget -= velocity;
				break;
			case FAST_RETREAT:
				m_distanceToTarget += 10.0f * velocity;
				break;
			case RETREAT:
				m_distanceToTarget += velocity;
				break;
		}
	}
	updateCameraVectors();
}

void Camera::setPositionAndTarget( const omath::dvec3 &pos, const omath::dvec3 &target ) {
	m_position = pos;
	m_target = target;
	calculateInitialAngles();
}

const float &Camera::getZoom() const {
	return m_zoom;
}

void Camera::setZoom( const float &zoom ) {
	m_zoom = zoom;
	calculateFOV();
}

void Camera::setUp( const omath::vec3 &up ) {
	m_up = up;
}

const float &Camera::getNearPlane() const {
	return m_nearPlane;
}

const float &Camera::getFarPlane() const {
	return m_farPlane;
}

void Camera::setNearPlane( const float &np ) {
	m_nearPlane = np;
	calculateFOV();
}

void Camera::setFarPlane( const float &fp ) {
	m_farPlane = fp;
	calculateFOV();
}

void Camera::setMode( const cameraMode mode ) {
	m_mode = mode;
}

void Camera::setMovementSpeed( const float &speed ) {
	m_movementSpeed = speed;
}

void Camera::printPosition() const {
	std::cout << "Cam position: " << m_position <<
			"\n    target: " << m_target <<
			"\n    direction: " << m_front <<
			"\n    mode: " << ( m_mode == FIRST_PERSON ? "first person\n" : "orbiting\n" );
}

const omath::mat4 &Camera::getViewMatrix() const {
	return m_viewMatrix;
}

/*const omath::mat4 &Camera::getZOffsetProjectionMatrix() const {
	return m_zOffsetProjectionMatrix;
}*/

const omath::mat4 &Camera::getPerspectiveMatrix() const {
	return m_perspectiveMatrix;
}

const omath::mat4 &Camera::getViewPerspectiveMatrix() const {
	return m_viewPerspectiveMatrix;
}

const omath::mat4 &Camera::getUntranslatedViewPerspectiveMatrix() const {
	return m_untranslatedViewPerspectiveMatrix;
}

const omath::dvec3 &Camera::getPosition() const {
	return m_position;
}

const float &Camera::getMovementSpeed() const {
	return m_movementSpeed;
}

const omath::dvec3 &Camera::getTarget() const {
	return m_target;
}

const bool &Camera::getWireframeMode() const {
	return m_wireframe;
}

const ViewFrustum &Camera::getViewFrustum() const {
	return m_frustum;
}

const omath::vec3 &Camera::getFront() const {
	return m_front;
}

const omath::vec3 &Camera::getRight() const {
	return m_right;
}

const omath::vec3 &Camera::getUp() const {
	return m_up;
}

Camera::~Camera() {
	deRegisterObject( this );
}

}
