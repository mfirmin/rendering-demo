#include "renderer.hpp"

#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"

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

    auto renderer = Renderer(width, height);

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    mesh->fromOBJ("assets/bunny.obj");

    std::unique_ptr<Material> material = std::make_unique<Material>();

    Model bunny(mesh, material);

    renderer.addModel(std::move(bunny));

    renderer.go();
}
