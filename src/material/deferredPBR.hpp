#pragma once

#include "material.hpp"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class DeferredPBRMaterial : public Material {
    public:
        DeferredPBRMaterial(glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0), float roughness = 0.5f, float metalness = 0.0f);
        ~DeferredPBRMaterial() = default;

        DeferredPBRMaterial(DeferredPBRMaterial&& other) = default;
        DeferredPBRMaterial(const DeferredPBRMaterial& other) = default;

        DeferredPBRMaterial& operator=(const DeferredPBRMaterial& other) = default;
        DeferredPBRMaterial& operator=(DeferredPBRMaterial&& other) = default;

        void create() override;

        void setRoughness(float roughness) const override;
        void setMetalness(float metalness) const override;

        void setLights(const std::vector<std::shared_ptr<Light>>& lights) const override { (void)lights; }
    private:
        float roughness;
        float metalness;
};
