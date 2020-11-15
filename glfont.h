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

	GLuint tex;		
	GLuint vbo;
	GLuint vao;

	unsigned int atlasWidth;			
	unsigned int atlasHeight;

	int vertexCount;

	// TODO: need a propar scaling according to the screen space
    float scaling;
    int fontSize;
    int outlineThickness;

    int screenWidth;
    int screenHeight;

    QString text;
    float xPos;
    float yPos;
    float xScale;
    float yScale;


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
    FT_Face face;

public:
    GLFont();
	~GLFont();
    void initialize(QOpenGLFunctions_3_3_Core* glFucntions, int width, int height);

    bool generateAtlas();

    void updateText();
	void renderText();

    bool setFont(const QString&);
    void setFontSize(int size);
    void setText(const QString& input, float x, float y, float sx, float sy);

    int getFontSize();
	float getScaling();
};

