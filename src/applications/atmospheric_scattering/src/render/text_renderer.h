
#ifndef TEXT_TEXT_RENDERER_H_
#define TEXT_TEXT_RENDERER_H_

#include "glad/glad.h"
#include <string>

class TextRenderer {
public:
	TextRenderer();
	TextRenderer(TextRenderer const&) = delete;
	TextRenderer(TextRenderer&&) = delete;
	~TextRenderer();

	void SetColor(float r, float g, float b);
	void DrawText(const std::string& text, int left, int top);

private:
	void SetupTexture();
	void SetupBuffers();
	void SetupProgram();
	void DrawChar(char c, int x, int y, int viewport_width, int viewport_height);

	GLuint font_texture_;
	GLuint char_vao_;
	GLuint char_vbo_;
	GLuint program_;
	GLfloat color_[3];
};

#endif  // TEXT_TEXT_RENDERER_H_
