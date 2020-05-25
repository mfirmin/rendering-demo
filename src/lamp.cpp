#include "lamp.hpp"

#include "light/light.hpp"
#include "light/pointLight.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"

#include <GL/glew.h>
#include <iostream>
#include <memory>

Lamp::Lamp(
    std::shared_ptr<Mesh> mesh,
    glm::vec3 position,
    glm::vec3 color,
    float intensity
) {
    std::unique_ptr<Material> material = std::make_unique<Material>(
        color,
        0.5f,
        8.0f
    );

    material->setEmissiveColorAndStrength(color, intensity);
    material->toggleEmissive(true);

    model = std::make_shared<Model>(mesh, std::move(material));
    model->setPosition(position);

    light = std::make_shared<PointLight>(
        position,
        color,
        intensity,
        0.0f,
        0.5f
    );

}

Lamp::~Lamp() {}

void Lamp::toggle() {
    light->toggle();

    if (active) {
        model->toggleEmissive(false);
    } else {
        model->toggleEmissive(true);
    }

    active = !active;
}

void Lamp::setColor(glm::vec3 color) {
    model->setColor(color);
    model->setEmissiveColor(color);
    light->setColor(color);
}

void Lamp::setIntensity(float i) {
    model->setEmissiveStrength(i);
    light->setIntensity(i);
}

void Lamp::setPosition(glm::vec3 p) {
    model->setPosition(p);
    light->setPosition(p);
}

void Lamp::setRotation(glm::vec3 r) {
    model->setRotation(r);
}

void Lamp::setScale(glm::vec3 s) {
    model->setScale(s);
}

void Lamp::setScale(float s) {
    model->setScale(s);
}

