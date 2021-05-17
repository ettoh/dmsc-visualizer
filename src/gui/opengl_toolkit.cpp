#include "opengl_toolkit.h"
#include <fstream>
#include <iostream>

namespace dmsc {

using std::string;

string OpenGLToolkit::readShader(const string& file_name) {
    std::ifstream file(file_name);
    if (!file)
        throw std::runtime_error("Shader '" + file_name + "' was not found!");

    string shader = "";
    string buffer;
    while (getline(file, buffer)) {
        shader.append(buffer + "\n");
    }

    return shader;
}

GLuint OpenGLToolkit::createShader(const std::string& file_name, GLenum shader_type) {
    GLint compile_flag = GL_FALSE;
    GLuint shader = glCreateShader(shader_type);

    // Read source code
    string sourcecode = readShader(file_name);
    const GLchar* source = sourcecode.c_str();
    glShaderSource(shader, 1, &source, NULL); // read an array of source files

    // Compile shader
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS,
                  &compile_flag); // get infos about the shader object and store it into compile_ok
    if (!compile_flag) {
        std::cout << "Error in compiling shader: '" + file_name + "'!" << std::endl;
    }

    return shader;
}

GLuint OpenGLToolkit::createProgram(const GLuint vertex_shader, const GLuint fragment_shader) {
    GLuint program = glCreateProgram();
    GLint link_ok = GL_FALSE;
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        // todo unified error handling
        throw std::runtime_error("Error in glLinkProgram");
    }

    return program;
}

} // namespace dmsc
