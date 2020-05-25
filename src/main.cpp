
#include "camera.hpp"
#include "light/directionalLight.hpp"
#include "light/pointLight.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "renderer.hpp"

#include <memory>
#include <stdlib.h>

constexpr uint32_t DEFAULT_WIDTH = 1600;
constexpr uint32_t DEFAULT_HEIGHT = 900;

int main(int argc, char* argv[]) {
    auto width = DEFAULT_WIDTH;
    auto height = DEFAULT_HEIGHT;

    if (argc == 3) {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
    }

    auto aspect = static_cast<float>(width) / height;

    auto sun = std::make_unique<DirectionalLight>(
        glm::vec3(-3.0f, 1.0f, -3.0f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        0.2f
    );

    auto sun2 = std::make_unique<DirectionalLight>(
        glm::vec3(3.0f, 3.0f, 1.0f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        0.2f
    );

    auto camera = std::make_unique<Camera>(aspect, 45.0f, -8.0f, glm::vec3(0.0f, 0.0f, 0.0f));

    auto renderer = Renderer(width, height, std::move(camera));

    // renderer.addLight(std::move(sun));
    // renderer.addLight(std::move(sun2));

    std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>();
    sphereMesh->fromOBJ("assets/sphere.obj");

    std::unique_ptr<Material> material = std::make_unique<Material>(
        glm::vec3(0.75164, 0.60648, 0.22648),
        0.5f,
        32.0f
    );

    Model sphere(sphereMesh, std::move(material));

    renderer.addModel(std::move(sphere));


    std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>();
    boxMesh->fromOBJ("assets/box.obj");

    std::unique_ptr<Material> boxMaterial = std::make_unique<Material>(
        glm::vec3(0.66, 0.66, 0.66),
        0.5f,
        8.0f
    );

    boxMaterial->setSide(Side::BACK);

    Model box(boxMesh, std::move(boxMaterial));
    renderer.addModel(std::move(box));

    std::unique_ptr<Material> lightMaterial = std::make_unique<Material>(
        glm::vec3(0.8f, 0.6f, 0.4f),
        0.5f,
        8.0f
    );

    lightMaterial->setEmissiveColorAndStrength(glm::vec3(0.8f, 0.6f, 0.4f), 1.0f);

    Model light(sphereMesh, std::move(lightMaterial));
    light.setScale(0.1f);
    light.setPosition(glm::vec3(-1.0f, -1.0f, -1.0f));

    renderer.addModel(std::move(light));

    auto lightLight = std::make_unique<PointLight>(
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(0.8f, 0.6f, 0.4f),
        0.0f,
        0.5f
    );

    renderer.addLight(std::move(lightLight));

    std::unique_ptr<Material> light2Material = std::make_unique<Material>(
        glm::vec3(0.2f, 0.9f, 0.5f),
        0.5f,
        8.0f
    );

    light2Material->setEmissiveColorAndStrength(glm::vec3(0.2f, 0.9f, 0.5f), 1.0f);

    Model light2(sphereMesh, std::move(light2Material));
    light2.setScale(0.1f);
    light2.setPosition(glm::vec3(1.0f, 1.0f, -1.0f));

    renderer.addModel(std::move(light2));

    auto light2Light = std::make_unique<PointLight>(
        glm::vec3(1.0f, 1.0f, -1.0f),
        glm::vec3(0.2f, 0.9f, 0.5f),
        0.0f,
        0.5f
    );

    renderer.addLight(std::move(light2Light));

    renderer.go();
}
