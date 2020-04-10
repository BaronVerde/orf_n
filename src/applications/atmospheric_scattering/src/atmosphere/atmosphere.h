
#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include "model.h"
#include "render/text_renderer.h"
//#include "render/program.h"

namespace scattering {

class atmosphere {
public:
	enum Luminance {
		NONE, APPROXIMATE, PRECOMPUTED
	};

	atmosphere();

	virtual ~atmosphere();

	void setup( int window_width, int window_height );

	void render();

	void cleanup();

	void key_event( int key, int action, int mods );

	void mouse_button_event( GLFWwindow* window, int button, int action, int mods );

	void mouse_move_event( double mouse_x, double mouse_y );

	void scroll_event( double xoffset, double yoffset );

private:

	void SetView( double view_distance_meters, double view_zenith_angle_radians,
			double view_azimuth_angle_radians, double sun_zenith_angle_radians,
			double sun_azimuth_angle_radians, double exposure );

	bool use_constant_solar_spectrum_;
	bool use_ozone_;
	bool use_combined_textures_;
	bool use_half_precision_;
	Luminance use_luminance_;
	bool do_white_balance_;
	bool show_help_;

	std::unique_ptr<Model> model_;
	GLuint vertex_shader_;
	GLuint fragment_shader_;
	GLuint program_;
	GLuint full_screen_quad_vao_;
	GLuint full_screen_quad_vbo_;
	std::unique_ptr<TextRenderer> text_renderer_;
	int window_id_;

	double view_distance_meters_;
	double view_zenith_angle_radians_;
	double view_azimuth_angle_radians_;
	double sun_zenith_angle_radians_;
	double sun_azimuth_angle_radians_;
	double exposure_;

	int window_width_;
	int window_height_;
	int previous_mouse_x_;
	int previous_mouse_y_;
	bool is_ctrl_key_pressed_;

};

}
