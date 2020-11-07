#pragma once
#include "ft2build.h"
#include FT_FREETYPE_H
#include <unordered_map>

#include "glm.hpp"
#include "shader.h"

#include <QOpenGLFunctions_3_3_Core>

class GLFont
{
private:
    QOpenGLFunctions_3_3_Core* gl;

    float outlineThickness;

	GLuint tex;		
	GLuint vbo;
	GLuint vao;

	unsigned int atlasWidth;			
	unsigned int atlasHeight;

	int vertexCount;

	// TODO: need a propar scaling according to the screen space
	float scaling;

	int lineHeight;

	struct Character {
		float advanceX;	
		float advanceY;

		float bitmapWidth;	
		float bitmapHeight;	

		float bitmapLeft;
		float bitmapTop;	

		float offsetX;	
		float offsetY;
	};

	std::unordered_map<char, Character> characters;

    FT_Library ft;

public:
    GLFont();
	~GLFont();
	void initialize(QOpenGLFunctions_3_3_Core* glFucntions);
    bool setFont(std::string& path);
    bool setFontSize(int size);
	bool generateAtlas(FT_Face face, int size, FT_Library ft);
    void updateText(std::string& input, float x, float y, float sx, float sy);
	void renderText();

	GLuint getTextureID();
	int getLineHeight();
	float getScaling();
};

