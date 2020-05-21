#pragma once

#include "light.hpp"

#include <glm/glm.hpp>

class PointLight : public Light {
    public:
        PointLight(
            glm::vec3 position,
            glm::vec3 color,
            float ambientCoefficient,
            float attenuation
        );

        PointLight(PointLight&& other) = default;
        PointLight& operator=(PointLight&& other) = default;

        PointLight(const PointLight& other) = default;
        PointLight& operator=(const PointLight& other) = default;

        ~PointLight() {};

        LightInfo getLightInfo() const override;
    private:
        glm::vec3 position;
};
