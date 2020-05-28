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

        virtual void create();

        virtual void setColor(glm::vec3 color);

        virtual void setShininess(float shininess);

        virtual void setLights(const std::vector<std::shared_ptr<Light>>& lights);

        virtual void setEmissiveColorAndStrength(glm::vec3 color, float strength);
        virtual void setEmissiveColor(glm::vec3 color);
        virtual void setEmissiveStrength(float strength);

        virtual void toggleEmissive(bool value);
        virtual void toggleBlinnPhongShading(bool value);

        virtual void setModelMatrix(const glm::mat4& modelMatrix);

        virtual void setProjectionAndViewMatrices(
            const glm::mat4& projectionMatrix,
            const glm::mat4& viewMatrix
        );

        virtual void setMatrices(
            const glm::mat4& projectionMatrix,
            const glm::mat4& viewMatrix,
            const glm::mat4& modelMatrix
        );

        bool compile(std::string vertexShader, std::string fragmentShader);

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
        GLuint program = 0;

        glm::vec3 color;
        float specularCoefficient;
        float shininess;

        Side side = Side::FRONT;
};
