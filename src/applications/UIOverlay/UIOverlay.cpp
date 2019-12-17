
#include "applications/Camera/Camera.h"	// near- and far plane
#include "UIOverlay.h"
#include "base/Globals.h"	// deltaTime
#include "base/Logbook.h"
#include "omath/mat4.h"
#include "renderer/Texture2D.h"
#include "renderer/Uniform.h"
#include "scene/Scene.h"
#include "imgui/imgui.h"

namespace orf_n {

extern double globals::deltaTime;
extern bool globals::showAppUI{ true };

// Low priority, rendered last
UIOverlay::UIOverlay( GlfwWindow *win ) :
		Renderable{ "UIOverlay" }, EventHandler{ win }, m_window{ win } {}

UIOverlay::~UIOverlay() {}

void UIOverlay::setup() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Setup style
	// ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();
	ImGuiStyle &style{ ImGui::GetStyle() };
	style.Alpha = 1.0f;
	io.DisplaySize.x = (float)m_window->getWidth();
	io.DisplaySize.y = (float)m_window->getHeight();

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
	io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	// Create the shaders
	std::vector<std::shared_ptr<Module>> modules;
	modules.push_back( std::make_shared<Module>( GL_VERTEX_SHADER,
			"Source/Applications/UIOverlay/UIOverlay.vert" ) );
	modules.push_back( std::make_shared<Module>( GL_FRAGMENT_SHADER,
			"Source/Applications/UIOverlay/UIOverlay.frag" ) );
	m_shader = new Program( modules );
	m_AttribLocationTex = glGetUniformLocation( m_shader->getProgram(), "Texture");
	m_AttribLocationPosition = glGetAttribLocation( m_shader->getProgram(), "Position");
	m_AttribLocationUV = glGetAttribLocation( m_shader->getProgram(), "UV");
	m_AttribLocationColor = glGetAttribLocation( m_shader->getProgram(), "Color");

	// Build texture atlas
    unsigned char* pixels;
    int width, height;
    // Load as RGBA 32-bits (75% of the memory is wasted, but default
    // font is so small) because it is more likely to be compatible with user's
    // existing shaders. If your ImTextureId represent a higher-level concept than
    // just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
    io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );
    // io.Fonts->GetTexDataAsAlpha8( &pixels, &width, &height );
    // Upload texture to graphics system
    glCreateTextures( GL_TEXTURE_2D, 1, &m_fontTexture);
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
    glTextureStorage2D( m_fontTexture, 1, GL_RGBA8, width, height );
    glTextureSubImage2D( m_fontTexture, 0,		// texture and mip level
    					 0, 0, width, height,	// offset and size
						 GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    glTextureParameteri( m_fontTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTextureParameteri( m_fontTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glBindTextureUnit( FONT_TEXTURE_UNIT, m_fontTexture );
    // Store our identifier
    m_shader->use();
    setUniform( m_shader->getProgram(), "Texture", FONT_TEXTURE_UNIT );
    m_shader->unUse();
    io.Fonts->TexID = (void *)(intptr_t)m_fontTexture;
    if( !io.Fonts->IsBuilt() )
    	Logbook::getInstance().logMsg( Logbook::RENDERER, Logbook::ERROR,
    			"The overlay font atlas was not built correctly." );

	glGenBuffers( 1, &m_vboHandle );
	glGenBuffers( 1, &m_elementsHandle );

	unsigned int cb = EventHandler::KEY |
					  EventHandler::MOUSE_MOVE |
					  EventHandler::MOUSE_BUTTON |
					  EventHandler::MOUSE_SCROLL |
					  EventHandler::CHAR;
	EventHandler::RegisteredObject o { this, cb };
	registerObject( o );

}

// Called here to enable Apps to use the UI in their render() method.
void UIOverlay::prepareFrame() {
	ImGui::NewFrame();
}

void UIOverlay::render() {
	if( !m_showUI ) {
		// An assertion would fail if the frame, begun in prepareFrame(), was not ended.
		ImGui::EndFrame();
		std::cout << "Frame ended\n";
		return;
	}
	// Backup GL state
	/* Setup render state: alpha-blending enabled, no face culling, no depth testing,
	 * scissor enabled, polygon fill */
	glEnable( GL_BLEND );
	glBlendEquation( GL_FUNC_ADD );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_SCISSOR_TEST );

	ImGuiIO& io{ ImGui::GetIO() };
	// get style for alpha blending
	ImGuiStyle &style{ ImGui::GetStyle() };
	if( io.DisplaySize.x != (float)m_window->getWidth() )
		io.DisplaySize.x = (float)m_window->getWidth();
	if( io.DisplaySize.y != (float)m_window->getHeight() )
		io.DisplaySize.y = (float)m_window->getHeight();

	io.DeltaTime = static_cast<float>( globals::deltaTime );

	omath::mat4 oMa = omath::ortho( 0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 0.0f, 1.0f );

	m_shader->use();
	setProjectionMatrix( oMa );

	ImGui::Begin( "orf_n UI" );
	ImGui::Text( "Press ctrl-U to toggle UI display on/off" );
	if( m_window->isCursorDisabled() )
		ImGui::Text( "Cursor disabled" );
	else
		ImGui::Text( "Cursor normal" );
	ImGui::SameLine(); ImGui::Text( "; left-ALT to toggle." );
	ImGui::Separator();
	bool oldVsync{ m_window->getVsync() };
	bool vsync{ oldVsync };
	ImGui::Checkbox( "Vsync", &vsync );
	if( vsync != oldVsync )
		m_window->setVsync( vsync );
	ImGui::SameLine(); ImGui::Text( "   %.1lf fps", 1 / globals::deltaTime );
	ImGui::SameLine();
	ImGui::Text( "FB size %d/%d", m_scene->getWindow()->getWidth(), m_scene->getWindow()->getHeight() );
	ImGui::SliderFloat( "UI alpha", &style.Alpha, 0.3f, 1.0f );
	ImGui::Separator();
	ImGui::Checkbox( "Show App UI", &globals::showAppUI );
	ImGui::End();
	ImGui::Render();

	/* Recreate the VAO every time
	 * This is to easily allow multiple GL contexts. VAO are not shared among GL contexts,
	 * and we don't track creation/deletion of windows so we don't have an obvious
	 * key to use to cache them. */
	GLuint vaoHandle = 0;
	glGenVertexArrays(1, &vaoHandle );
	glBindVertexArray( vaoHandle );
	glBindBuffer( GL_ARRAY_BUFFER, m_vboHandle );
	glEnableVertexAttribArray( m_AttribLocationPosition );
	glEnableVertexAttribArray( m_AttribLocationUV );
	glEnableVertexAttribArray( m_AttribLocationColor );
	glVertexAttribPointer( m_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE,
			sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos) );
	glVertexAttribPointer( m_AttribLocationUV, 2, GL_FLOAT, GL_FALSE,
			sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv) );
	glVertexAttribPointer( m_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,
			sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col) );

	/* TODO: Setup render state: alpha-blending enabled, no face culling, no depth testing,
	 * scissor enabled. Setup viewport, orthographic projection matrix Setup shader:
	 * vertex { float2 pos, float2 uv, u32 color }, fragment shader sample color from 1 texture,
	 * multiply by vertex color. */
	ImDrawData *drawData = ImGui::GetDrawData();
	ImVec2 pos = drawData->DisplayPos;
	int fbWidth{ 0 };
	int fbHeight{ 0 };
	glfwGetFramebufferSize( m_window->getWindow(), &fbWidth, &fbHeight );
	for( int n = 0; n < drawData->CmdListsCount; n++ ) {
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;
		glBindBuffer( GL_ARRAY_BUFFER, m_vboHandle );
		glBufferData( GL_ARRAY_BUFFER,
				(GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
				(const GLvoid*)cmd_list->VtxBuffer.Data,
				GL_STREAM_DRAW );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_elementsHandle );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,
				(GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
				(const GLvoid*)cmd_list->IdxBuffer.Data,
				GL_STREAM_DRAW );
		for( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++ ) {
             const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
             // User callback (registered via ImDrawList::AddCallback)
             if( pcmd->UserCallback ) {
            	 pcmd->UserCallback( cmd_list, pcmd );
             } else {
            	 ImVec4 clip_rect = ImVec4( pcmd->ClipRect.x - pos.x,
            			 	 	 	 	 	pcmd->ClipRect.y - pos.y,
											pcmd->ClipRect.z - pos.x,
											pcmd->ClipRect.w - pos.y );
            	 if( (int)clip_rect.x < fbWidth &&
            		 (int)clip_rect.y < fbHeight &&
					 clip_rect.z >= 0.0f &&
					 clip_rect.w >= 0.0f ) {
            		 // Apply scissor/clipping rectangle
            		 glScissor( (int)clip_rect.x,
            				 	fbHeight - (int)clip_rect.w,
								(int)(clip_rect.z - clip_rect.x),
								(int)(clip_rect.w - clip_rect.y) );
            		 // Bind texture, Draw
            		 //glBindTexture( GL_TEXTURE_2D, m_fontTexture );
            		 glDrawElements( GL_TRIANGLES,
            				 	 	 (GLsizei)pcmd->ElemCount,
									 sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
									 idx_buffer_offset );
            	 }
             }
             idx_buffer_offset += pcmd->ElemCount;
		}
	}
	glDeleteVertexArrays(1, &vaoHandle);

	glDisable( GL_SCISSOR_TEST );
}

void UIOverlay::cleanup() {
	deRegisterObject( this );
	glDeleteBuffers( 1, &m_vboHandle );
	glDeleteBuffers( 1, &m_elementsHandle );
	glDeleteTextures( 1, &m_fontTexture );
	delete m_shader;
	ImGui::DestroyContext();
}

// functions to be called by the EventHandler if UIOverlay is present
// virtual
bool UIOverlay::onMouseMove( float x, float y ) {
	if( m_window->isCursorDisabled() )
		return false;
    ImGuiIO& io = ImGui::GetIO();
    GLFWwindow *win = m_window->getWindow();
	// Update mouse position
    const ImVec2 mouse_pos_backup = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    if( glfwGetWindowAttrib( win, GLFW_FOCUSED ) ) {
    	if( io.WantSetMousePos ) {
    		glfwSetCursorPos( win, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
    	} else {
    		double mouse_x, mouse_y;
    		glfwGetCursorPos( win, &mouse_x, &mouse_y);
    		io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    	}
    }
	return true;
}

// virtual
bool UIOverlay::onMouseButton( int button, int action, int mods ) {
	if( m_window->isCursorDisabled() )
		return false;
	bool handled = false;
	bool mouseJustPressed[5] { false, false, false, false, false };
	if( GLFW_PRESS == action && button >= 0 && button < IM_ARRAYSIZE( mouseJustPressed ) ) {
		mouseJustPressed[button] = true;
		handled = true;
	}
	// Update buttons
	ImGuiIO& io = ImGui::GetIO();
	for( int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++ ) {
		// If a mouse press event came, always pass it as "mouse held this frame",
		// so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[i] = mouseJustPressed[i] || glfwGetMouseButton( m_window->getWindow(), i ) != 0;
		mouseJustPressed[i] = false;
	}
	return handled;
}

// virtual
bool UIOverlay::onMouseScroll( float xOffset, float yOffset ) {
	return true;
}

// virtual
bool UIOverlay::onKeyPressed( int key, int scancode, int action, int mods ) {
	// Toggle UI with ctrl-U
	bool handled{ false };
	if( GLFW_PRESS == action && GLFW_KEY_U == key && ( GLFW_MOD_CONTROL & mods ) ) {
		m_showUI = !m_showUI;
		handled = true;
	}
	return handled;
}

// virtual
bool UIOverlay::onChar( unsigned int code ) {
	if( code > 0 && code < 0x10000 ) {
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter( (unsigned short)code );
	}
	return false;
}

} /* namespace orf_n */
