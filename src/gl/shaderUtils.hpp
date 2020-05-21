#pragma once

#include <GL/glew.h>

namespace ShaderUtils {
    GLuint compile(const GLchar* vertexShaderSource[], const GLchar* fragmentShaderSource[]);
} /* ShaderUtils */
