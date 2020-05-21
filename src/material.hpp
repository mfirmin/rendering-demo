#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Light;

class Material {
    public:
        Material(glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0));
        ~Material();

        Material(Material&& other) = default;
        Material(const Material& other) = default;

        Material& operator=(const Material& other) = default;
        Material& operator=(Material&& other) = default;

        void setColor(glm::vec3 color);

        void setLights(const std::vector<std::unique_ptr<Light>>& lights);

        void setProjectionAndViewMatrices(
            const glm::mat4& projectionMatrix,
            const glm::mat4& viewMatrix
        );

        void setMatrices(
            const glm::mat4& projectionMatrix,
            const glm::mat4& viewMatrix,
            const glm::mat4& modelMatrix
        );

        GLuint getProgram() {
            return program;
        }
    private:
        GLuint program;
};
