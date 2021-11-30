#include "deferredPBR.hpp"

#include "gl/shaderUtils.hpp"

#include "light/light.hpp"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <sstream>

DeferredPBRMaterial::DeferredPBRMaterial(
    glm::vec3 color,
    float roughness,
    float metalness
) :
    Material(color, 0.0f, 0.0f),
    roughness(roughness),
    metalness(metalness)
{ }

void DeferredPBRMaterial::create() {
    if (getProgram() != 0) {
        // already initialized
        return;
    }
    std::string vertexShaderSource = R"(
        #version 330
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;
        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;

        out vec3 vNormalEyespace;
        out vec4 vPositionEyespace;

        void main() {
            vNormalEyespace = (transpose(inverse(viewMatrix * modelMatrix)) * vec4(normal, 0.0)).xyz;
            vPositionEyespace = viewMatrix * modelMatrix * vec4(position, 1.0);

            gl_Position = projectionMatrix * vPositionEyespace;
        }
    )";

    std::string fragmentShaderSource = R"(
        #version 330

        layout(location = 0) out vec4 position;
        layout(location = 1) out vec3 normal;
        layout(location = 2) out vec4 albedo;
        layout(location = 3) out vec4 emissive;
        layout(location = 4) out vec2 roughnessAndMetalness;

        uniform vec3 color;
        uniform float specularCoefficient;

        // todo: shininess?
        // uniform float shininess;

        uniform vec3 emissiveColor;
        uniform float emissiveStrength;
        uniform float emissiveEnabled;

        uniform float roughness;
        uniform float metalness;

        in vec3 vNormalEyespace;
        in vec4 vPositionEyespace;

        void main() {
            vec3 N = normalize(vNormalEyespace);
            vec3 E = normalize(-vPositionEyespace.xyz);

            if (dot(N, E) < 0.0) {
                N = -N;
            }

            position = vPositionEyespace;
            normal = N;
            albedo = vec4(color, specularCoefficient);

            roughnessAndMetalness = vec2(roughness, metalness);

            if (emissiveEnabled > 0.5) {
                emissive = vec4(emissiveColor, emissiveStrength);
            }
        }
    )";

    if (!compile(vertexShaderSource, fragmentShaderSource)) {
        return;
    }

    GLuint shader = getProgram();

    glUseProgram(shader);
    auto projectionMatrixLocation = glGetUniformLocation(shader, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(shader, "viewMatrix");
    auto modelMatrixLocation = glGetUniformLocation(shader, "modelMatrix");
    auto colorLocation = glGetUniformLocation(shader, "color");
    auto specularCoefficientLocation = glGetUniformLocation(shader, "specularCoefficient");
    auto emissiveColorLocation = glGetUniformLocation(shader, "emissiveColor");
    auto emissiveStrengthLocation = glGetUniformLocation(shader, "emissiveStrength");
    auto emissiveEnabledLocation = glGetUniformLocation(shader, "emissiveEnabled");

    auto roughnessLocation = glGetUniformLocation(shader, "roughness");
    auto metalnessLocation = glGetUniformLocation(shader, "metalness");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniform3fv(colorLocation, 1, glm::value_ptr(getColor()));
    glUniform1f(specularCoefficientLocation, getSpecularCoefficient());
    glUniform3fv(emissiveColorLocation, 1, glm::value_ptr(getColor()));
    glUniform1f(emissiveStrengthLocation, 0.0f);
    glUniform1f(emissiveEnabledLocation, 0.0f);

    glUniform1f(roughnessLocation, roughness);
    glUniform1f(metalnessLocation, metalness);
    glUseProgram(0);
}

void DeferredPBRMaterial::setRoughness(float roughness) const {
    GLuint shader = getProgram();
    glUseProgram(shader);

    auto roughnessLocation = glGetUniformLocation(shader, "roughness");
    glUniform1f(roughnessLocation, roughness);
    glUseProgram(0);
}

void DeferredPBRMaterial::setMetalness(float metalness) const {
    GLuint shader = getProgram();
    glUseProgram(shader);

    auto metalnessLocation = glGetUniformLocation(shader, "metalness");
    glUniform1f(metalnessLocation, metalness);
    glUseProgram(0);
}
