
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
        glm::vec3(1.0f, 1.0f, 1.0f),
        0.2f
    );

    auto spark = std::make_unique<PointLight>(
        glm::vec3(0.45f, -0.2f, -1.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        0.0f,
        0.0f
    );

    auto camera = std::make_unique<Camera>(aspect, 45.0f, -8.0f, glm::vec3(0.0f, 0.0f, 0.0f));

    auto renderer = Renderer(width, height, std::move(camera));

    renderer.addLight(std::move(sun));
    renderer.addLight(std::move(spark));

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    mesh->fromOBJ("assets/sphere.obj");

    std::unique_ptr<Material> material = std::make_unique<Material>(
        glm::vec3(0.75164, 0.60648, 0.22648),
        0.5f,
        32.0f
    );

    Model sphere(std::move(mesh), std::move(material));

    renderer.addModel(std::move(sphere));


    std::unique_ptr<Mesh> boxMesh = std::make_unique<Mesh>();
    boxMesh->fromOBJ("assets/box.obj");

    std::unique_ptr<Material> boxMaterial = std::make_unique<Material>(
        glm::vec3(0.66, 0.66, 0.66),
        0.5f,
        8.0f
    );

    boxMaterial->setSide(Side::BACK);

    Model box(std::move(boxMesh), std::move(boxMaterial));
    renderer.addModel(std::move(box));

    renderer.go();
}
