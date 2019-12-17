
/**
 * Class for ui overlay, derived from imgui glfw example.
 * Weird castings of font texture id keep me from integrating this
 * into my own render classes.
 */

#pragma once

#include "base/GlfwWindow.h"
#include "renderer/Program.h"
#include "scene/Renderable.h"
#include "imgui/imgui.h"

namespace orf_n {

class UIOverlay : public Renderable, EventHandler {
public:
	UIOverlay( GlfwWindow *win );

	virtual ~UIOverlay();

	void setup() override final;

	void prepareFrame() override final;

	void render() override final;

	void cleanup() override final;

	UIOverlay &operator=( const UIOverlay &eh ) = delete;

	UIOverlay( const UIOverlay &eh ) = delete;

private:
	/**
	 * @brief Shader unit the font texture is bound to and not rebound !
	 * @note When creating textures, leave this texture binding point untouched.
	 */
	static constexpr GLint FONT_TEXTURE_UNIT { 20 };

	bool m_showUI{ true };

	GlfwWindow *m_window;

	Program *m_shader;

	GLuint m_fontTexture;

	/**
	 * Global alpha for the UI. Must not be 0 or UI becomes invisible !
	 */
	float m_uiAlpha{ 0.6f };

	int m_AttribLocationTex;
	int m_AttribLocationPosition;
	int m_AttribLocationUV;
	int m_AttribLocationColor;

	GLuint m_vboHandle;

	GLuint m_elementsHandle;

	virtual bool onMouseMove( float x, float y ) override;

	virtual bool onMouseButton( int button, int action, int mods ) override;

	virtual bool onMouseScroll( float xOffset, float yOffset ) override;

	virtual bool onKeyPressed( int key, int scancode, int action, int mods ) override;

	virtual bool onChar( unsigned int code ) override;

};

}
