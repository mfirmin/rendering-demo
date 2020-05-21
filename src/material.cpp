#include "material.hpp"

#include "gl/shaderUtils.hpp"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

Material::Material(glm::vec3 color) {
    const GLchar* vertexShaderSource[] = {
        R"(
        #version 330
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;
        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;

        out vec3 vNormalEyespace;

        void main() {
            vNormalEyespace = (transpose(inverse(viewMatrix * modelMatrix)) * vec4(normal, 0.0)).xyz;
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
        }
        )"
    };

    const GLchar* fragmentShaderSource[] = {
        R"(
        #version 330

        out vec4 fColor;

        uniform vec3 color;

        in vec3 vNormalEyespace;

        vec3 lightsource = vec3(0.0, 1.0, 1.0);

        void main() {
            vec3 N = normalize(vNormalEyespace);
            vec3 L = normalize(lightsource);

            float d = max(0.0, dot(N, L));
            float a = 0.2;

            fColor = vec4((d + a) * color, 1.0);
        }
    )"
    };

    program = ShaderUtils::compile(vertexShaderSource, fragmentShaderSource);

    if (program == 0) {
        return;
    }

    glUseProgram(program);
    auto projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    auto modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");
    auto colorLocation = glGetUniformLocation(program, "color");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glUseProgram(0);
}

Material::~Material() {}

void Material::setColor(glm::vec3 color) {
    glUseProgram(program);
    auto colorLocation = glGetUniformLocation(program, "color");
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glUseProgram(0);
}

void Material::setProjectionAndViewMatrices(
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix
) {
    glUseProgram(program);
    auto projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUseProgram(0);
}

void Material::setMatrices(
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
) {
    glUseProgram(program);
    auto projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    auto modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUseProgram(0);
}
