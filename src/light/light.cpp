#include "light.hpp"

Light::Light(
    glm::vec3 color,
    float ambientCoefficient,
    float attenuation
) :
    color(color),
    ambientCoefficient(ambientCoefficient),
    attenuation(attenuation)
{}
