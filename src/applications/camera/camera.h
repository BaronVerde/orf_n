
/**
 * A class for a 3D alround camera, orbiting or firsrt person.
 */

#pragma once

#include <base/event_handler.h>
#include "geometry/ViewFrustum.h"
#include "omath/mat4.h"

namespace orf_n {

class glfw_window;
class Plane;
class ViewFrustum;

class camera : public event_handler {
public:
	// Terrain is a first person camera with double precision view matrix
	typedef enum {
		ORBITING = 0, FIRST_PERSON, TERRAIN
	} cameraMode;

	typedef enum {
		// Movement of first person camera
		FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN, ROTATE_LEFT, ROTATE_RIGHT,
		FAST_FORWARD, FAST_BACKWARD, FAST_RIGHT, FAST_LEFT, FAST_UP, FAST_DOWN,
		// Movemtn of orbiting camera
		CLOSE, RETREAT, FAST_CLOSE, FAST_RETREAT
	} direction;

	/**
	 * Common camera object, will be stored in the scene for all renderable objects
	 * to access it. Needs pointer to window, position and target as double vectors,
	 * camera up vector (default y up), near and far plane as floats and the mode,
	 * if it is an orbiting camera around the target or a free moving first person type.
	 */
	camera( glfw_window *win,
			omath::dvec3 position,
			omath::dvec3 target,
			omath::vec3 up = omath::vec3{ 0.0f, 1.0f, 0.0f },
			float neaPlane = 1.0f,
			float farPlane = 100.0f,
			cameraMode mode = ORBITING );

	virtual ~camera();

	const omath::mat4 &getViewMatrix() const;

	const omath::mat4 &getViewPerspectiveMatrix() const;

	const omath::mat4 &getUntranslatedViewPerspectiveMatrix() const;

	const ViewFrustum &getViewFrustum() const;

	const omath::vec3 &getFront() const;

	const omath::vec3 &getRight() const;

	const omath::vec3 &getUp() const;

	const omath::mat4 &getPerspectiveMatrix() const;

	const omath::mat4 &getZOffsetProjMatrix() const;

	const omath::dvec3 &getPosition() const;

	const omath::dvec3 &getTarget() const;

	void setPositionAndTarget( const omath::dvec3 &pos, const omath::dvec3 &target );

	void setMode( const cameraMode mode );

	void setUp( const omath::vec3 &up );

	const float &getMovementSpeed() const;

	void setMovementSpeed( const float &speed );

	/**
	 * Zoom angle of camera in degrees
	 */
	void setZoom( const float &zoom );

	const float &getZoom() const ;

	const bool &getWireframeMode() const;

	/**
	 * Must be called on near/far plane or angle change.
	 */
	void calculateFOV();

	/**
	 * Update camera and movement. Must be called every frame for continous movemnt.
	 */
	void updateMoving();

	const float &get_near_plane() const;

	const float &get_far_plane() const;

	void set_near_plane( const float &np );

	void set_far_plane( const float &fp );

private:
	glfw_window *m_window;

	omath::dvec3 m_position;

	/**
	 * Orbiting camera target.
	 */
	omath::dvec3 m_target{ omath::vec3{ 0.0 } };

	double m_distanceToTarget;

	float m_nearPlane{ 10.0 };

	float m_farPlane{ 10000.0 };

	float m_zoom{ 45.0f };

	ViewFrustum m_frustum;

	omath::mat4 m_viewMatrix;

	/**
	 * The scene's projection matrix.
	 * A renderable object can have its own projection matrix.
	 * Moved here from the scene for better access for frustum calculation.
	 */
	omath::mat4 m_perspectiveMatrix;

	/**
	 * Perspective projection * view Matrix pre-comp.
	 */
	omath::mat4 m_viewPerspectiveMatrix;

	omath::mat4 m_untranslatedViewPerspectiveMatrix;

	//omath::mat4 m_zOffsetPerspectiveMatrix;

	omath::vec3 m_front;

	omath::vec3 m_up{ omath::vec3{ 0.0f, 1.0f, 0.0f } };

	// omath::vec3 m_worldUp; // for physics

	omath::vec3 m_right;

	float m_yaw{ 0.0f };

	float m_pitch{ 0.0f };

	cameraMode m_mode{ ORBITING };

	float m_movementSpeed{ 30.0f };

	/**
	  Moving starts with key press, ends with key release.
	 */
	bool m_isMoving{ false };

	/**
	 * Movement direction if camera is moving in firstperson mode, relative to camera axes.
	 */
	direction m_direction;

	float m_mouseSensitivity{ 0.02f };

	/**
	 * Center of screeen (width and height/2) for relative mouse movement.
	 * @fixme: this should be unnecessary to carry along. doublecheck glfw3 documentation.
	 */
	omath::vec2 m_centerOfScreen;

	/**
	 * Switch between wireframe and textured display. Used internally.
	 */
	bool m_wireframe{ false };

	bool on_key_pressed( int key, int scancode, int action, int mods ) override final;

	bool on_mouse_move( float x, float y ) override final;

	bool on_mouse_button( int button, int action, int mods ) override final;

	bool on_mouse_scroll( float xOffset, float yOffset ) override final;

	bool on_framebuffer_resize( int width, int height ) override final;

	/**
	 * Helper function to calculate distance, yaw and pitch on creation and
	 * when position or target have been changed.
	 */
	void calculateInitialAngles();

	/**
	 * Helper func calculates cam vectors on key press or mous move, depending on mode.
	 * Depend on prior calculateInitialValues() on camera creation or change of
	 * position or target !
	 */
	void updatecameraVectors();

	// debug output
	void printPosition() const;

};

}	// namespace
