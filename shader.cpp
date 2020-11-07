#include "shader.h"
#include <iostream>

Shader::Shader()
{

}

void Shader::initialize(QOpenGLFunctions_3_3_Core* glFunctions, const char* vertexSource, const char* fragmentSource)
{
	gl = glFunctions;

    unsigned int vertex, fragment;

    //TODO: quick shader compiler withtout error check
    vertex = gl->glCreateShader(GL_VERTEX_SHADER);
    gl->glShaderSource(vertex, 1, &vertexSource, NULL);
    gl->glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");    

    fragment = gl->glCreateShader(GL_FRAGMENT_SHADER);
    gl->glShaderSource(fragment, 1, &fragmentSource, NULL);
    gl->glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    ID = gl->glCreateProgram();
    gl->glAttachShader(ID, vertex);
    gl->glAttachShader(ID, fragment);
    gl->glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    gl->glDeleteShader(vertex);
    gl->glDeleteShader(fragment);
}

void Shader::setUniformMat4(const std::string& name, const glm::mat4& mat)
{
    gl->glUniformMatrix4fv(gl->glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniformVec3(const std::string& name, const glm::vec3& vec)
{
    gl->glUniform3fv(gl->glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
}

void Shader::setUniformVec4(const std::string& name, const glm::vec4& vec)
{
    gl->glUniform3fv(gl->glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
}

void Shader::setUniformInt(const std::string& name, int value) {
    gl->glUniform1i(gl->glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::bind()
{
    gl->glUseProgram(ID);
}

void Shader::unbind()
{
    gl->glUseProgram(0);
}

void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            gl->glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        gl->glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            gl->glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
