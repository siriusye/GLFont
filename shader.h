#pragma once
#include <string>
#include <QOpenGLFunctions_3_3_Core>
#include "glm.hpp"

class Shader
{
private:
	QOpenGLFunctions_3_3_Core* gl;
public:
	unsigned int ID;

	Shader();
	void initialize(QOpenGLFunctions_3_3_Core* glFunctions, const char* vertexSource, const char* fragmentSource);
	void setUniformMat4(const std::string& name, const glm::mat4& mat);
	void setUniformVec3(const std::string& name, const glm::vec3& vec);
	void setUniformVec4(const std::string& name, const glm::vec4& vec);
	void setUniformInt(const std::string& name, int value);
 
	void bind();
	void unbind();
	void checkCompileErrors(GLuint shader, std::string type);
};

