
#include "atmosphere.h"
#include <cmath>
#include <sstream>
#include <fstream>

namespace scattering {

constexpr double kPi = 3.1415926;
constexpr double kSunAngularRadius = 0.00935 / 2.0;
constexpr double kSunSolidAngle = kPi * kSunAngularRadius * kSunAngularRadius;
constexpr double kLengthUnitInMeters = 1000.0;

const char kVertexShader[] = R"(
    #version 330
    uniform mat4 model_from_view;
    uniform mat4 view_from_clip;
    layout(location = 0) in vec4 vertex;
    out vec3 view_ray;
    void main() {
      view_ray =
          (model_from_view * vec4((view_from_clip * vertex).xyz, 0.0)).xyz;
      gl_Position = vertex;
    })";

#include "demo.glsl.inc"

atmosphere::atmosphere() :
		use_constant_solar_spectrum_(false), use_ozone_(true), use_combined_textures_(true),
		use_half_precision_(true), use_luminance_(NONE), do_white_balance_(false), show_help_(true),
		program_(0), view_distance_meters_(9000.0), view_zenith_angle_radians_(1.47), view_azimuth_angle_radians_(-0.1),
		sun_zenith_angle_radians_(1.3), sun_azimuth_angle_radians_(2.9), exposure_(10.0) {
	glGenVertexArrays(1, &full_screen_quad_vao_);
	glBindVertexArray(full_screen_quad_vao_);
	glGenBuffers(1, &full_screen_quad_vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, full_screen_quad_vbo_);
	const GLfloat vertices[] = {
			-1.0, -1.0, 0.0, 1.0,
			+1.0, -1.0, 0.0, 1.0,
			-1.0, +1.0, 0.0, 1.0,
			+1.0, +1.0, 0.0, 1.0,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	constexpr GLuint kAttribIndex = 0;
	constexpr int kCoordsPerVertex = 4;
	glVertexAttribPointer(kAttribIndex, kCoordsPerVertex, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(kAttribIndex);
	glBindVertexArray(0);
	text_renderer_.reset(new TextRenderer);
}

atmosphere::~atmosphere() {
	// TODO Auto-generated destructor stub
}

void atmosphere::setup( int window_width, int window_height ) {
	window_width_ = window_width;
	window_height_ = window_height;
	constexpr int kLambdaMin = 360;
	constexpr int kLambdaMax = 830;
	constexpr double kSolarIrradiance[48] = {
			1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
			1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
			1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
			1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
			1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
			1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
	};
	constexpr double kOzoneCrossSection[48] = {
			1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
			8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
			1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
			4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
			2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
			6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
			2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
	};
	// From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
	constexpr double kDobsonUnit = 2.687e20;
	constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
	constexpr double kConstantSolarIrradiance = 1.5;
	constexpr double kBottomRadius = 6360000.0;
	constexpr double kTopRadius = 6420000.0;
	constexpr double kRayleigh = 1.24062e-6;
	constexpr double kRayleighScaleHeight = 8000.0;
	constexpr double kMieScaleHeight = 1200.0;
	constexpr double kMieAngstromAlpha = 0.0;
	constexpr double kMieAngstromBeta = 5.328e-3;
	constexpr double kMieSingleScatteringAlbedo = 0.9;
	constexpr double kMiePhaseFunctionG = 0.8;
	constexpr double kGroundAlbedo = 0.1;
	const double max_sun_zenith_angle = (use_half_precision_ ? 102.0 : 120.0) / 180.0 * M_PI;
	DensityProfileLayer rayleigh_layer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
	DensityProfileLayer mie_layer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
	std::vector<DensityProfileLayer> ozone_density;
	ozone_density.push_back( DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
	ozone_density.push_back( DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));
	std::vector<double> wavelengths;
	std::vector<double> solar_irradiance;
	std::vector<double> rayleigh_scattering;
	std::vector<double> mie_scattering;
	std::vector<double> mie_extinction;
	std::vector<double> absorption_extinction;
	std::vector<double> ground_albedo;
	for (int l = kLambdaMin; l <= kLambdaMax; l += 10) {
		double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
		double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
		wavelengths.push_back(l);
		if (use_constant_solar_spectrum_)
			solar_irradiance.push_back(kConstantSolarIrradiance);
		else
			solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
		rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
		mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
		mie_extinction.push_back(mie);
		absorption_extinction.push_back(use_ozone_ ?
				kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10] : 0.0);
		ground_albedo.push_back(kGroundAlbedo);
	}
	model_.reset( new Model( wavelengths, solar_irradiance, kSunAngularRadius, kBottomRadius,
			kTopRadius, {rayleigh_layer}, rayleigh_scattering, {mie_layer}, mie_scattering,
			mie_extinction, kMiePhaseFunctionG, ozone_density, absorption_extinction, ground_albedo,
			max_sun_zenith_angle, kLengthUnitInMeters,
			use_luminance_ == PRECOMPUTED ? 15 : 3, use_combined_textures_, use_half_precision_));
	model_->Init();
	vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
	const char* const vertex_shader_source = kVertexShader;
	std::ofstream f( "demo.vs.glsl", std::ios::out );
	f << vertex_shader_source << std::endl;
	f.close();
	glShaderSource(vertex_shader_, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader_);
	const std::string fragment_shader_str =
			"#version 330\n" + std::string(use_luminance_ != NONE ? "#define USE_LUMINANCE\n" : "") +
			"const float kLengthUnitInMeters = " + std::to_string(kLengthUnitInMeters) + ";\n" + demo_glsl;
	const char* fragment_shader_source = fragment_shader_str.c_str();
	fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
	f.open( "demo.fs.glsl", std::ios::out );
	f << fragment_shader_source << std::endl;
	f.close();
	glShaderSource(fragment_shader_, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader_);
	if (program_ != 0)
		glDeleteProgram(program_);
	program_ = glCreateProgram();
	glAttachShader(program_, vertex_shader_);
	glAttachShader(program_, fragment_shader_);
	glAttachShader(program_, model_->shader());
	glLinkProgram(program_);
	glDetachShader(program_, vertex_shader_);
	glDetachShader( program_, fragment_shader_ );
	glDetachShader( program_, model_->shader() );
	glUseProgram(program_);
	model_->SetProgramUniforms(program_, 0, 1, 2, 3);
	double white_point_r = 1.0;
	double white_point_g = 1.0;
	double white_point_b = 1.0;
	if (do_white_balance_) {
		Model::ConvertSpectrumToLinearSrgb(
				wavelengths, solar_irradiance, &white_point_r, &white_point_g, &white_point_b
		);
		double white_point = (white_point_r + white_point_g + white_point_b) / 3.0;
		white_point_r /= white_point;
		white_point_g /= white_point;
		white_point_b /= white_point;
	}
	glUniform3f(glGetUniformLocation(program_, "white_point"), white_point_r, white_point_g, white_point_b);
	glUniform3f(glGetUniformLocation(program_, "earth_center"), 0.0, 0.0, -kBottomRadius / kLengthUnitInMeters);
	glUniform2f(glGetUniformLocation(program_, "sun_size"), tan(kSunAngularRadius), cos(kSunAngularRadius));
	// This sets 'view_from_clip', which only depends on the window size.
	glViewport(0, 0, window_width, window_height);
	const float kFovY = 50.0f / 180.0f * M_PI;
	const float kTanFovY = tan(kFovY / 2.0f);
	float aspect_ratio = static_cast<float>(window_width) / window_height;
	// Transform matrix from clip space to camera space (i.e. the inverse of a GL_PROJECTION matrix).
	float view_from_clip[16] = {
			kTanFovY * aspect_ratio, 0.0, 0.0, 0.0,
			0.0, kTanFovY, 0.0, 0.0,
			0.0, 0.0, 0.0, -1.0,
			0.0, 0.0, 1.0, 1.0
	};
	glUniformMatrix4fv(glGetUniformLocation(program_, "view_from_clip"), 1, true, view_from_clip);
}

void atmosphere::render() {
	// Unit vectors of the camera frame, expressed in world space.
	float cos_z = cos(view_zenith_angle_radians_);
	float sin_z = sin(view_zenith_angle_radians_);
	float cos_a = cos(view_azimuth_angle_radians_);
	float sin_a = sin(view_azimuth_angle_radians_);
	float ux[3] = { -sin_a, cos_a, 0.0 };
	float uy[3] = { -cos_z * cos_a, -cos_z * sin_a, sin_z };
	float uz[3] = { sin_z * cos_a, sin_z * sin_a, cos_z };
	float l = view_distance_meters_ / kLengthUnitInMeters;
	// Transform matrix from camera frame to world space (i.e. the inverse of a GL_MODELVIEW matrix).
	float model_from_view[16] = {
			ux[0], uy[0], uz[0], uz[0] * l,
			ux[1], uy[1], uz[1], uz[1] * l,
			ux[2], uy[2], uz[2], uz[2] * l,
			0.0, 0.0, 0.0, 1.0
	};
	glUniform3f(
			glGetUniformLocation(program_, "camera"), model_from_view[3], model_from_view[7], model_from_view[11]
	);
	glUniform1f(glGetUniformLocation(program_, "exposure"), use_luminance_ != NONE ? exposure_ * 1e-5 : exposure_);
	glUniformMatrix4fv(glGetUniformLocation(program_, "model_from_view"), 1, true, model_from_view);
	glUniform3f(glGetUniformLocation(program_, "sun_direction"),
			cos(sun_azimuth_angle_radians_) * sin(sun_zenith_angle_radians_),
			sin(sun_azimuth_angle_radians_) * sin(sun_zenith_angle_radians_),
			cos(sun_zenith_angle_radians_)
	);
	glBindVertexArray(full_screen_quad_vao_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	if (show_help_) {
		std::stringstream help;
		help << "Mouse:\n" << " drag, CTRL+drag, wheel: view and sun directions\n" << "Keys:\n" <<
				" h: help\n" << " s: solar spectrum (currently: " <<
				(use_constant_solar_spectrum_ ? "constant" : "realistic") << ")\n" <<
				" o: ozone (currently: " << (use_ozone_ ? "on" : "off") << ")\n" <<
				" t: combine textures (currently: " << (use_combined_textures_ ? "on" : "off") << ")\n" <<
				" p: half precision (currently: " << (use_half_precision_ ? "on" : "off") << ")\n" <<
				" l: use luminance (currently: " <<
				(use_luminance_ == PRECOMPUTED ? "precomputed" : (use_luminance_ == APPROXIMATE ? "approximate" : "off")) <<
				")\n"  << " w: white balance (currently: "  << (do_white_balance_ ? "on" : "off") << ")\n" <<
				" +/-: increase/decrease exposure (" << exposure_ << ")\n" << " 1-9: predefined views\n";
		text_renderer_->SetColor(1.0, 0.0, 0.0);
		text_renderer_->DrawText(help.str(), 5, 4);
	}
}

void atmosphere::cleanup() {}

void atmosphere::key_event( int key, int action, int mods ) {
	if( GLFW_PRESS == action ) {
		switch( key ) {
			case GLFW_KEY_H: show_help_ = !show_help_; break;
			case GLFW_KEY_S:
				use_constant_solar_spectrum_ = !use_constant_solar_spectrum_;
				setup( window_width_, window_height_ );
				break;
			case GLFW_KEY_O:
				use_ozone_ = !use_ozone_;
				setup( window_width_, window_height_ );
				break;
			case GLFW_KEY_T:
				use_combined_textures_ = !use_combined_textures_;
				setup( window_width_, window_height_ );
				break;
			case GLFW_KEY_P:
				use_half_precision_ = !use_half_precision_;
				setup( window_width_, window_height_ );
				break;
			case GLFW_KEY_L:
				switch (use_luminance_) {
					case NONE: use_luminance_ = APPROXIMATE; break;
					case APPROXIMATE: use_luminance_ = PRECOMPUTED; break;
					case PRECOMPUTED: use_luminance_ = NONE; break;
				};
				setup( window_width_, window_height_ );
				break;
			case GLFW_KEY_W:
				do_white_balance_ = !do_white_balance_;
				setup( window_width_, window_height_ );
				break;
			case GLFW_KEY_COMMA: exposure_ *= 1.1; break;
			case GLFW_KEY_PERIOD: exposure_ /= 1.1; break;
			case GLFW_KEY_1: SetView( 9000.0, 1.47, 0.0, 1.3, 3.0, 10.0); break;
			case GLFW_KEY_2: SetView( 9000.0, 1.47, 0.0, 1.564, -3.0, 10.0); break;
			case GLFW_KEY_3: SetView( 7000.0, 1.57, 0.0, 1.54, -2.96, 10.0); break;
			case GLFW_KEY_4: SetView( 7000.0, 1.57, 0.0, 1.328, -3.044, 10.0); break;
			case GLFW_KEY_5: SetView( 9000.0, 1.39, 0.0, 1.2, 0.7, 10.0); break;
			case GLFW_KEY_6: SetView( 9000.0, 1.5, 0.0, 1.628, 1.05, 200.0); break;
			case GLFW_KEY_7: SetView( 7000.0, 1.43, 0.0, 1.57, 1.34, 40.0); break;
			case GLFW_KEY_8: SetView( 2.7e6, 0.81, 0.0, 1.57, 2.0, 10.0); break;
			case GLFW_KEY_9: SetView( 1.2e7, 0.0, 0.0, 0.93, -2.0, 10.0); break;
			case GLFW_KEY_LEFT_CONTROL : is_ctrl_key_pressed_ = !is_ctrl_key_pressed_; break;
		}
	}
}

void atmosphere::mouse_button_event( GLFWwindow* window, int button, int action, int mods ) {};

void atmosphere::mouse_move_event( double mouse_x, double mouse_y ) {
	constexpr double kScale = 500.0;
	if( is_ctrl_key_pressed_ ) {
		sun_zenith_angle_radians_ -= (previous_mouse_y_ - mouse_y) / kScale;
		sun_zenith_angle_radians_ = std::max(0.0, std::min(kPi, sun_zenith_angle_radians_));
		sun_azimuth_angle_radians_ += (previous_mouse_x_ - mouse_x) / kScale;
	} else {
		view_zenith_angle_radians_ += (previous_mouse_y_ - mouse_y) / kScale;
		view_zenith_angle_radians_ = std::max(0.0, std::min(kPi / 2.0, view_zenith_angle_radians_));
		view_azimuth_angle_radians_ += (previous_mouse_x_ - mouse_x) / kScale;
	}
	previous_mouse_x_ = mouse_x;
	previous_mouse_y_ = mouse_y;
}

void atmosphere::scroll_event( double xoffset, double yoffset ) {
	if( yoffset > 0.0 )
		view_distance_meters_ *= 1.05;
	else
		view_distance_meters_ /= 1.05;
}

void atmosphere::SetView( double view_distance_meters, double view_zenith_angle_radians,
		double view_azimuth_angle_radians, double sun_zenith_angle_radians,
		double sun_azimuth_angle_radians, double exposure) {
	view_distance_meters_ = view_distance_meters;
	view_zenith_angle_radians_ = view_zenith_angle_radians;
	view_azimuth_angle_radians_ = view_azimuth_angle_radians;
	sun_zenith_angle_radians_ = sun_zenith_angle_radians;
	sun_azimuth_angle_radians_ = sun_azimuth_angle_radians;
	exposure_ = exposure;
}

}
