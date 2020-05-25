
#include "camera.hpp"
#include "lamp.hpp"
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
        1.0f,
        0.2f
    );

    auto sun2 = std::make_unique<DirectionalLight>(
        glm::vec3(3.0f, 3.0f, 1.0f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        1.0f,
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

    std::shared_ptr<Model> sphere = std::make_shared<Model>(sphereMesh, std::move(material));

    renderer.addModel(sphere);


    std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>();
    boxMesh->fromOBJ("assets/box.obj");

    std::unique_ptr<Material> boxMaterial = std::make_unique<Material>(
        glm::vec3(0.66, 0.66, 0.66),
        0.5f,
        8.0f
    );

    boxMaterial->setSide(Side::BACK);

    std::shared_ptr<Model> box = std::make_shared<Model>(boxMesh, std::move(boxMaterial));
    renderer.addModel(box);


    Lamp lamp1(sphereMesh, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.8f, 0.6f, 0.4f), 1.0f);
    lamp1.setScale(0.1f);

    renderer.addModel(lamp1.getModel());
    renderer.addLight(lamp1.getLight());

    Lamp lamp2(sphereMesh, glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.2f, 0.9f, 0.5f), 1.0f);
    lamp2.setScale(0.1f);

    renderer.addModel(lamp2.getModel());
    renderer.addLight(lamp2.getLight());

    Lamp lamp3(sphereMesh, glm::vec3(0.0f, 0.1f, 1.5f), glm::vec3(0.2f, 0.2f, 0.9f), 0.8f);
    lamp3.setScale(0.2f);

    renderer.addModel(lamp3.getModel());
    renderer.addLight(lamp3.getLight());

    Lamp lamp4(sphereMesh, glm::vec3(-1.0f, 1.5f, 1.5f), glm::vec3(0.9f, 0.2f, 0.1f), 2.0f);
    lamp4.setScale(0.05f);

    renderer.addModel(lamp4.getModel());
    renderer.addLight(lamp4.getLight());

    renderer.go();
}
