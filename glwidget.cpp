#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_3_Core>
#include <QtDebug>
#include <math.h>
#include <iostream>

#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    renderText = true;
}

GLWidget::~GLWidget()
{
    delete glFont;
    delete textShader;
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

static const char* textVert = R"END(
#version 330 core
layout(location = 0) in vec4 vertex; 
out vec2 texCoords; 
void main() 
{
    gl_Position = vec4(vertex.xy, 0.0, 1.0); 
    texCoords = vertex.zw; 
};
)END";

static const char* textFragOutline = R"END(
#version 330 core
in vec2 texCoords;
out vec4 color; 
uniform sampler2D text; 
uniform vec3 textColor; 
uniform vec3 outlineColor; 

void main()
{ 
    vec2 tex = texture2D(text, texCoords).rg; 
    float fill = tex.r;
    float outline = tex.g;
    float alpha = max(fill, outline);
    vec3 mix_color = mix(mix(vec3(0.0), textColor, fill), outlineColor, outline);
    color = vec4(mix_color, alpha);
};
)END";


static const char* textFrag = R"END(
#version 330 core
in vec2 texCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)END";

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    gl = (QOpenGLFunctions_3_3_Core*)QOpenGLContext::currentContext()->versionFunctions();

    textShader = new Shader();
    textShader->initialize(gl, textVert, textFrag);
    textShader->bind();
    textShader->setUniformVec3("textColor", glm::vec3(0.0, 0.5, 1.0));
    // textShader->setUniformVec3("outlineColor", glm::vec3(1.0, 0.5, 0.3));

    glFont = new GLFont();
    glFont->initialize(gl, this->width(), this->height());

    gl->glClearColor(0.811f, 0.886f, 0.953f, 1);
    gl->glEnable(GL_CULL_FACE);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    QString welcome = "Welcome";
    glFont->setText(welcome, -1, 1 - glFont->getFontSize() * glFont->getScaling(), 1.0, 1.0);

}

void GLWidget::paintGL()
{
    gl->glClear(GL_COLOR_BUFFER_BIT);

    if (renderText)
    {
        textShader->bind();
        glFont->renderText();
    }
}

void GLWidget::resizeGL(int w, int h)
{

}

void GLWidget::updateText(const QString& text)
{
    if (text.size() == 0)
    {
        renderText = false;
        update();
        return;
    }
    else
    {
        renderText = true;
    }

    setUpdatesEnabled(false);

    glFont->setText(text, -1, 1 - glFont->getFontSize() * glFont->getScaling(), 1.0, 1.0);

    setUpdatesEnabled(true);
    update();
}

void GLWidget::updateFont(const QString& path)
{
    glFont->setFont(path);
    glFont->updateText();
}

void GLWidget::toggleOutline(bool outline)
{
    if(outline)
    {
        textShader->initialize(gl, textVert, textFragOutline);
        textShader->bind();
        textShader->setUniformVec3("textColor", glm::vec3(0.0, 0.5, 1.0));
        textShader->setUniformVec3("outlineColor", glm::vec3(1.0, 0.5, 0.3));
    }
    else
    {
        textShader->initialize(gl, textVert, textFrag);
        textShader->bind();
        textShader->setUniformVec3("textColor", glm::vec3(0.0, 0.5, 1.0));
    }
    update();
}
