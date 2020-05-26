
#include "camera.hpp"
#include "lamp.hpp"
#include "light/directionalLight.hpp"
#include "light/pointLight.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "renderer.hpp"

#include <chrono>
#include <GL/glew.h>
#include <memory>
#include <stdlib.h>

constexpr float ONE_SECOND = 1000.0f;
constexpr float FPS = 60.0f;

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

    auto sun = std::make_shared<DirectionalLight>(
        glm::vec3(-3.0f, 1.0f, -3.0f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        1.0f,
        0.2f
    );

    auto sun2 = std::make_shared<DirectionalLight>(
        glm::vec3(3.0f, 3.0f, 1.0f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        1.0f,
        0.2f
    );

    auto camera = std::make_unique<Camera>(aspect, 45.0f, -8.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    auto renderer = Renderer(width, height, std::move(camera));

    sun->toggle();
    sun2->toggle();

    renderer.addLight(sun);
    renderer.addLight(sun2);

    std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>();
    sphereMesh->fromOBJ("assets/sphere.obj");

    std::unique_ptr<Material> material = std::make_unique<Material>(
        glm::vec3(0.75164, 0.60648, 0.22648),
        0.5f,
        64.0f
    );

    std::shared_ptr<Model> sphere = std::make_shared<Model>(sphereMesh, std::move(material));

    renderer.addModel(sphere);


    std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>();
    boxMesh->fromOBJ("assets/box.obj");

    std::unique_ptr<Material> boxMaterial = std::make_unique<Material>(
        glm::vec3(0.66, 0.66, 0.66),
        0.5f,
        4.0f
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

    Lamp lamp4(sphereMesh, glm::vec3(-1.0f, 1.5f, 1.5f), glm::vec3(0.9f, 0.2f, 0.1f), 1.0f);
    lamp4.setScale(0.05f);

    renderer.addModel(lamp4.getModel());
    renderer.addLight(lamp4.getLight());

    float frameLength = ONE_SECOND / FPS;
    auto last = std::chrono::steady_clock::now();
    bool quit = false;

    bool mouseDown = false;
    while (!quit) {
        auto now = std::chrono::steady_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();

        if (dt >= frameLength) {
            // handleEvents(quit);
            if (quit) {
                break;
            }

            // handle events

            SDL_Event e;

            while(SDL_PollEvent(&e) != 0) {
                if (
                    e.type == SDL_QUIT ||
                    (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                ) {
                    quit = true;
                    break;
                } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    mouseDown = true;
                } else if (e.type == SDL_MOUSEMOTION) {
                    int x = 0, y = 0;
                    if (mouseDown) {
                        SDL_GetRelativeMouseState(&x, &y);
                        renderer.updateCameraRotation(glm::vec3(-static_cast<float>(y) / 100.0f, -static_cast<float>(x) / 100.0f, 0.0f));
                    } else {
                        SDL_GetRelativeMouseState(&x, &y);
                    }
                } else if (e.type == SDL_MOUSEBUTTONUP) {
                    mouseDown = false;
                } else if (e.type == SDL_KEYUP) {
                    auto key = std::string(SDL_GetKeyName(e.key.keysym.sym));
                    if (key == "A") {
                        renderer.toggleMSAA();
                    } else if (key == "1") {
                        lamp1.toggle();
                    } else if (key == "2") {
                        lamp2.toggle();
                    } else if (key == "3") {
                        lamp3.toggle();
                    } else if (key == "4") {
                        lamp4.toggle();
                    } else if (key == "L") {
                        // Primary Lighting
                        sun->toggle();
                        sun2->toggle();
                    } else if (key == "B") {
                        renderer.toggleBlinnPhongShading();
                    }
                }
            }

            if (quit) {
                break;
            }

            renderer.render();
            last = now;
        }

    }
}
