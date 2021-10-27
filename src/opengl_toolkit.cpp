#include "opengl_toolkit.hpp"
#include <cassert>
#include <fstream>

#include <iostream>

namespace dmsc {
namespace tools {

std::string readShader(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file) {
        printf("Failed to load shader %s\n", file_name.c_str());
        assert(false);
        exit(EXIT_FAILURE);
    }

    std::string shader = "";
    std::string buffer;
    while (getline(file, buffer)) {
        shader.append(buffer + "\n");
    }

    return shader;
}

// ------------------------------------------------------------------------------------------------

GLuint createShader(const std::string& file_name, GLenum shader_type) {
    GLint compile_flag = GL_FALSE;
    GLuint shader = glCreateShader(shader_type);

    // Read source code
    std::string sourcecode = readShader(file_name);
    const GLchar* source = sourcecode.c_str();
    glShaderSource(shader, 1, &source, NULL); // read an array of source files

    // Compile shader
    glCompileShader(shader);
    glGetShaderiv(shader,
                  GL_COMPILE_STATUS,
                  &compile_flag); // get infos about the shader object and store it into compile_ok
    if (!compile_flag) {
        debugShaderObject(shader);
        printf("Error in compiling shader: '%s'!\n", file_name.c_str());
        assert(false);
        exit(EXIT_FAILURE);
    }

    return shader;
}

// ------------------------------------------------------------------------------------------------

void debugShaderObject(const GLuint shader_object) {
    GLint log_length = 0;
    if (glIsShader(shader_object)) {
        glGetShaderiv(shader_object, GL_INFO_LOG_LENGTH, &log_length);
    } else {
        std::cerr << "The given object is not a shader!" << std::endl;
        return;
    }

    char* log = (char*)malloc(log_length);
    glGetShaderInfoLog(shader_object, log_length, NULL, log);
    std::cerr << log;
    free(log);
}

// ------------------------------------------------------------------------------------------------

GLuint createProgram(const GLuint vertex_shader, const GLuint fragment_shader) {
    GLuint program = glCreateProgram();
    GLint link_ok = GL_FALSE;
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        printf("Error in glLinkProgram\n");
        assert(false);
        exit(EXIT_FAILURE);
    }

    return program;
}

} // namespace tools
} // namespace dmsc
