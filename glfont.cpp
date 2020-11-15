#include "glfont.h"
#include <string>
#include <iostream>
#include "freetype/ftglyph.h"
#include "freetype/ftstroke.h"

GLFont::GLFont()
{
    outlineThickness = 1;
    scaling = 0.01f;
    fontSize = 42;
}

void GLFont::setFontSize(int size)
{
    this->fontSize = size;
    generateAtlas();
    updateText();
}

int GLFont::getFontSize()
{
    return fontSize;
}

float GLFont::getScaling()
{
    return scaling;
}

bool GLFont::setFont(const QString& path)
{
    if (FT_New_Face(ft, path.toStdString().c_str(), 0, &face)) {
        std::cout << "Failed to create face for loaded font" << std::endl;
        return false;
    }
    else {
        generateAtlas();
    }

    return true;
}

// TODO: rework on process
void GLFont::initialize(QOpenGLFunctions_3_3_Core* glFucntions, int width, int height)
{
    gl = glFucntions;

    if (FT_Init_FreeType(&ft))
    {
        std::cout << "Could not init FreeType" << std::endl;
        return;
    }

    gl->glGenBuffers(1, &vbo);
    gl->glGenVertexArrays(1, &vao);
    gl->glBindVertexArray(vao);

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);

    gl->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    gl->glEnableVertexAttribArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl->glBindVertexArray(0);
    vertexCount = 0;

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glGenTextures(1, &tex);

    screenWidth = width;
    screenHeight = height;

    QString path("font/OpenSans-Regular.ttf");
    setFont(path);
}

bool GLFont::generateAtlas()
{
    if (FT_Set_Pixel_Sizes(face, 0, fontSize))
    {
        return false;
    }

    unsigned int rowWidth = 0;
    unsigned int rowHeight = 0;
    atlasWidth = 0;
    atlasHeight = 0;

    FT_Glyph glyphOutline;
    FT_Stroker stroker;
    FT_BitmapGlyph glyphBitmap;
    FT_Bitmap bitmapStroke;
    unsigned int bitmapWidth = 0;
    unsigned int bitmapHeight = 0;

    std::unordered_map<char, std::vector<unsigned char>> buffers;

    // first pass create outline and calculate the altas size 
    for (unsigned int i = 32; i < 127; i++) {

        if (FT_Load_Char(face, i, FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL) != 0)
        {
            std::cout << "load char faild" << std::endl;
            return false;
        }

        if (FT_Get_Glyph(face->glyph, &glyphOutline) != 0)
        {
            std::cout << "get glyph faild" << std::endl;
            return false;
        }

        if (FT_Stroker_New(ft, &stroker) != 0)
        {
            std::cout << "stroker new faild" << std::endl;
            return false;
        }

        FT_Stroker_Set(stroker, static_cast<FT_Fixed>(outlineThickness * static_cast<float>(1 << 6)), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
        if (FT_Glyph_Stroke(&glyphOutline, stroker, true) != 0)
        {
            std::cout << "glyph stroke faild" << std::endl;
            return false;
        }

        if (FT_Glyph_To_Bitmap(&glyphOutline, FT_RENDER_MODE_NORMAL, 0, 1) != 0)
        {
            std::cout << "glyph to bitmap faild" << std::endl;
            return false;
        }

        glyphBitmap = (FT_BitmapGlyph)glyphOutline;
        bitmapStroke = glyphBitmap->bitmap;

        bitmapWidth = bitmapStroke.width;
        bitmapHeight = bitmapStroke.rows;
       
        buffers[i] = std::vector<unsigned char>(bitmapWidth * bitmapHeight * 2, 0);
        for (unsigned int j = 0; j < bitmapWidth * bitmapHeight; j++)
        {
            buffers[i][j * 2 + 1] = bitmapStroke.buffer[j];  
        }

        characters[i].bitmapWidth = bitmapWidth;
        characters[i].bitmapHeight = bitmapHeight;

        characters[i].advanceX = face->glyph->advance.x >> 6;
        characters[i].advanceY = face->glyph->advance.y >> 6;
        
        characters[i].bitmapLeft = glyphBitmap->left;
        characters[i].bitmapTop = glyphBitmap->top;
       
        if (rowWidth + bitmapWidth + 1 >= screenWidth) {
            atlasWidth = atlasWidth > rowWidth ? atlasWidth : rowWidth;
            atlasHeight += rowHeight;
            rowWidth = 0;
            rowHeight = 0;
        }
        rowWidth += bitmapWidth + 1;
        rowHeight = rowHeight > bitmapHeight ? rowHeight : bitmapHeight;
    }
    FT_Done_Glyph(glyphOutline);

    atlasWidth = atlasWidth > rowWidth ? atlasWidth : rowWidth;
    atlasHeight += rowHeight;
    
    gl->glBindTexture(GL_TEXTURE_2D, tex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, atlasWidth, atlasHeight, 0, GL_RG, GL_UNSIGNED_BYTE, 0);

    gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int offsetX = 0;
    int offsetY = 0;

    rowHeight = 0;

    FT_Glyph glyphFill;
    // second pass create fill and atlas texture
    for (int i = 32; i < 127; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER) != 0) {
            return false;
        }

        if (offsetX + characters[i].bitmapWidth + 1 >= screenWidth) {
            offsetY += rowHeight;
            rowHeight = 0;
            offsetX = 0;
        }

        FT_Get_Glyph(face->glyph, &glyphFill);
        FT_Glyph_To_Bitmap(&glyphFill, FT_RENDER_MODE_NORMAL, 0, 1);

        glyphBitmap = (FT_BitmapGlyph)glyphFill;
        bitmapStroke = glyphBitmap->bitmap;

        unsigned int bitmapWidth = bitmapStroke.width;
        unsigned int bitmapHeight = bitmapStroke.rows;
        unsigned int innerOffsetX = (characters[i].bitmapWidth - bitmapWidth) / 2;  
        unsigned int innerOffsetY = (characters[i].bitmapHeight - bitmapHeight) / 2;

        characters[i].offsetX = offsetX / (float)atlasWidth;
        characters[i].offsetY = offsetY / (float)atlasHeight;
        
        // TODO: find a way to do copy based allocation
        for (unsigned int y = 0; y < bitmapHeight; ++y)
        {
            for (unsigned int x = 0; x < bitmapWidth; ++x)
            {
                unsigned int source = y * bitmapWidth + x;
                unsigned int target = (y + innerOffsetX) * characters[i].bitmapWidth + x + innerOffsetY;

                buffers[i][target * 2] = bitmapStroke.buffer[source];
            }
        }

        gl->glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, characters[i].bitmapWidth, characters[i].bitmapHeight, GL_RG, GL_UNSIGNED_BYTE, buffers[i].data());
       
        rowHeight = rowHeight > characters[i].bitmapHeight ? rowHeight : characters[i].bitmapHeight;
        offsetX += characters[i].bitmapWidth + 1;
     }

    FT_Done_Glyph(glyphFill);

    return true;
}

void GLFont::setText(const QString& input, float x, float y, float sx, float sy)
{
    this->text = input;
    this->xPos = x;
    this->yPos = y;
    this->xScale = sx;
    this->yScale = sy;
    updateText();
}

void GLFont::updateText()
{
    std::vector<glm::vec4> coordinates;
    coordinates.resize(6* text.length());

    float x = xPos;
    float y = yPos;
    float sx = xScale;
    float sy = yScale;

    sx *= scaling;
    sy *= scaling;

    for (int i = 0; i < text.length(); i++)
    {
        // TODO: find check contain
        Character c = characters[text.toStdString().at(i)];

        if (x + c.advanceX * sx> 1.0)
        {
            x = -1;
            y -= fontSize * scaling;
        }

        float x2 = x + c.bitmapLeft * sx;
        float y2 = -y - c.bitmapTop * sy;
        float w = c.bitmapWidth * sx;
        float h = c.bitmapHeight * sy;

        x += c.advanceX * sx;

        if (!w || !h)
            continue;

        coordinates[i * 6] = glm::vec4(x2, -y2 - h, c.offsetX, c.offsetY + c.bitmapHeight/ atlasHeight);
        coordinates[i * 6 + 2] = glm::vec4(x2, -y2, c.offsetX, c.offsetY);
        coordinates[i * 6 + 1] = glm::vec4(x2 + w, -y2, c.offsetX + c.bitmapWidth / atlasWidth, c.offsetY);

        coordinates[i * 6 +3] = glm::vec4(x2 + w, -y2, c.offsetX + c.bitmapWidth/ atlasWidth, c.offsetY);
        coordinates[i * 6 +4] = glm::vec4(x2, -y2 - h, c.offsetX, c.offsetY + c.bitmapHeight / atlasHeight);
        coordinates[i * 6 +5] = glm::vec4(x2 + w, -y2 - h, c.offsetX + c.bitmapWidth / atlasWidth, c.offsetY + c.bitmapHeight / atlasHeight);
    }

    gl->glBindVertexArray(vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl->glBufferData(GL_ARRAY_BUFFER, coordinates.size() * sizeof(coordinates[0]), &coordinates[0], GL_DYNAMIC_DRAW);

    vertexCount = coordinates.size();
}

void GLFont::renderText()
{
    gl->glBindTexture(GL_TEXTURE_2D, tex);
    gl->glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

GLFont::~GLFont()
{
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    gl->glDeleteTextures(1, &tex);
    gl->glDeleteBuffers(1, &vbo);
    gl->glDeleteVertexArrays(1, &vao);
}
