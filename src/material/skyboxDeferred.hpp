#pragma once

#include "material.hpp"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class SkyboxDeferredMaterial : public Material {
    public:
        SkyboxDeferredMaterial(GLuint cubemap);
        ~SkyboxDeferredMaterial() = default;

        SkyboxDeferredMaterial(SkyboxDeferredMaterial&& other) = default;
        SkyboxDeferredMaterial(const SkyboxDeferredMaterial& other) = default;

        SkyboxDeferredMaterial& operator=(const SkyboxDeferredMaterial& other) = default;
        SkyboxDeferredMaterial& operator=(SkyboxDeferredMaterial&& other) = default;

        void create() override;

        void setColor(glm::vec3 color) const override { (void)color; }

        void setShininess (float shininess) const override { (void)shininess; }

        void setLights (const std::vector<std::shared_ptr<Light>>& lights) const override { (void)lights; }

        void setEmissiveColorAndStrength(glm::vec3 color, float strength) const override { (void) color; (void)strength; }
        void setEmissiveColor(glm::vec3 color) const override { (void)color; }
        void setEmissiveStrength(float strength) const override { (void)strength; }

        void toggleEmissive(bool value) const override { (void)value; }
        void toggleBlinnPhongShading(bool value) const override { (void)value; }

        void setModelMatrix(const glm::mat4& modelMatrix) const override { (void)modelMatrix; }

        void setUniforms() const override;

    private:
        GLuint cubemap;
};
