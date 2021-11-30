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
        virtual ~Material();

        Material(Material&& other) = default;
        Material(const Material& other) = default;

        Material& operator=(const Material& other) = default;
        Material& operator=(Material&& other) = default;

        virtual void create();

        virtual void setColor(glm::vec3 color) const;

        virtual void setShininess(float shininess) const;

        virtual void setLights(const std::vector<std::shared_ptr<Light>>& lights) const;

        virtual void setEmissiveColorAndStrength(glm::vec3 color, float strength) const;
        virtual void setEmissiveColor(glm::vec3 color) const;
        virtual void setEmissiveStrength(float strength) const;

        virtual void toggleEmissive(bool value) const;
        virtual void toggleBlinnPhongShading(bool value) const;

        virtual void setModelMatrix(const glm::mat4& modelMatrix) const;

        virtual void setMetalness(float metalness) const { (void)metalness; }
        virtual void setRoughness(float roughness) const { (void)roughness; }

        virtual void setProjectionAndViewMatrices(
            const glm::mat4& projectionMatrix,
            const glm::mat4& viewMatrix
        ) const;

        virtual void setMatrices(
            const glm::mat4& projectionMatrix,
            const glm::mat4& viewMatrix,
            const glm::mat4& modelMatrix
        ) const;

        virtual void setUniforms() const {}

        bool compile(std::string vertexShader, std::string fragmentShader);

        GLuint getProgram() const {
            return program;
        }

        void setSide(Side s) {
            side = s;
        }

        Side getSide() const {
            return side;
        }

        glm::vec3 getColor() const {
            return color;
        }

        float getSpecularCoefficient() const {
            return specularCoefficient;
        }

        float getShininess() const {
            return shininess;
        }
    private:
        GLuint program = 0;

        glm::vec3 color;
        float specularCoefficient;
        float shininess;

        Side side = Side::FRONT;
};
