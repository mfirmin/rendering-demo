#include "pointLight.hpp"

PointLight::PointLight(
    glm::vec3 position,
    glm::vec3 color,
    float ambientCoefficient,
    float attenuation
) :
    Light(color, ambientCoefficient, attenuation),
    position(position)
{}

LightInfo PointLight::getLightInfo() const {
    return {
        glm::vec4(position, 1.0f),
        color,
        ambientCoefficient,
        attenuation,
        0.0f,
        glm::vec3(0.0f)
    };
}
