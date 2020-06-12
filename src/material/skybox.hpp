#pragma once

#include "material.hpp"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class SkyboxMaterial : public Material {
    public:
        SkyboxMaterial(GLuint cubemap);
        virtual ~SkyboxMaterial();

        SkyboxMaterial(SkyboxMaterial&& other) = default;
        SkyboxMaterial(const SkyboxMaterial& other) = default;

        SkyboxMaterial& operator=(const SkyboxMaterial& other) = default;
        SkyboxMaterial& operator=(SkyboxMaterial&& other) = default;

        void create() override;

        void setColor(glm::vec3 color) override { (void)color; }

        void setShininess (float shininess) override { (void)shininess; }

        void setLights (const std::vector<std::shared_ptr<Light>>& lights) override { (void)lights; }

        void setEmissiveColorAndStrength(glm::vec3 color, float strength) override { (void) color; (void)strength; }
        void setEmissiveColor(glm::vec3 color) override { (void)color; }
        void setEmissiveStrength(float strength) override { (void)strength; }

        void toggleEmissive(bool value) override { (void)value; }
        void toggleBlinnPhongShading(bool value) override { (void)value; }

        void setModelMatrix(const glm::mat4& modelMatrix) override { (void)modelMatrix; }

        void setUniforms() override;

    private:
        GLuint cubemap;
};
