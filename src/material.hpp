#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

enum class Side { FRONT, BACK, BOTH };

class Light;

class Material {
    public:
        Material(glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0), float specularCoefficient = 0.5f, float shininess = 32.0f);
        ~Material();

        Material(Material&& other) = default;
        Material(const Material& other) = default;

        Material& operator=(const Material& other) = default;
        Material& operator=(Material&& other) = default;

        void setColor(glm::vec3 color);

        void setShininess(float shininess);

        void setLights(const std::vector<std::shared_ptr<Light>>& lights);

        void setEmissiveColorAndStrength(glm::vec3 color, float strength);
        void setEmissiveColor(glm::vec3 color);
        void setEmissiveStrength(float strength);

        void toggleEmissive(bool value);
        void toggleBlinnPhongShading(bool value);

        void setModelMatrix(const glm::mat4& modelMatrix);

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

        void setSide(Side s) {
            side = s;
        }

        Side getSide() {
            return side;
        }
    private:
        GLuint program;

        Side side = Side::FRONT;
};
