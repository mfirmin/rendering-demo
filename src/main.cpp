#include "renderer.hpp"

#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"

#include <iostream>
#include <memory>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    auto width = 1600;
    auto height = 900;
//    auto width = 800;
//    auto height = 450;
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
