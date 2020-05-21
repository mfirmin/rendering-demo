#include "directionalLight.hpp"

DirectionalLight::DirectionalLight(
    glm::vec3 direction,
    glm::vec3 color,
    float ambientCoefficient
) :
    Light(color, ambientCoefficient, 0.0f),
    direction(direction)
{}

LightInfo DirectionalLight::getLightInfo() const {
    return {
        glm::vec4(direction, 0.0f),
        color,
        ambientCoefficient,
        attenuation,
        0.0f,
        glm::vec3(0.0f)
    };
}
