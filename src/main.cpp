
#include "camera.hpp"
#include "lamp.hpp"
#include "light/directionalLight.hpp"
#include "light/pointLight.hpp"
#include "material/material.hpp"
#include "material/deferredMaterial.hpp"
#include "material/deferredPBR.hpp"
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
        glm::vec3(1.0f, 1.0f, 1.0f),
        0.5f,
        0.2f
    );

    auto sun2 = std::make_shared<DirectionalLight>(
        glm::vec3(3.0f, 3.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        0.5f,
        0.2f
    );

    auto camera = std::make_unique<Camera>(aspect, 45.0f, -8.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    auto renderer = Renderer(width, height, std::move(camera));

    sun->toggle();
    sun2->toggle();

    renderer.addLight(sun);
    renderer.addLight(sun2);

    // std::shared_ptr<Mesh> teapotMesh = std::make_shared<Mesh>();
    // teapotMesh->fromOBJ("assets/teapot.obj");

    std::shared_ptr<Mesh> bunnyMesh = std::make_shared<Mesh>();
    bunnyMesh->fromOBJ("assets/bunny.obj");

    std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>();
    sphereMesh->fromOBJ("assets/sphere.obj");

    std::unique_ptr<Material> material = std::make_unique<Material>(
        glm::vec3(0.75164, 0.60648, 0.22648),
        0.5f,
        64.0f
    );

    std::unique_ptr<Material> deferredMaterial = std::make_unique<DeferredMaterial>(
        glm::vec3(0.75164, 0.60648, 0.22648),
        1.0f,
        64.0f
    );

    std::unique_ptr<Material> pbrMaterial = std::make_unique<DeferredPBRMaterial>(
        glm::vec3(1.00, 0.71, 0.29), // gold
        // glm::vec3(0.91, 0.92, 0.92), // aluminum
        // glm::vec3(0.95, 0.93, 0.88), // silver
        0.2f,
        1.0f
    );

    // std::shared_ptr<Model> teapot = std::make_shared<Model>(teapotMesh, std::move(material));
    // teapot->setScale(0.1f);
    // teapot->setRotation(glm::vec3(-1.57f, 0.0f, 0.0f));
    // teapot->setPosition(glm::vec3(0.0f, -0.75f, 0.0f));

    std::shared_ptr<Model> bunny = std::make_shared<Model>(bunnyMesh, std::move(material));
    bunny->setPosition(glm::vec3(0.3f, -1.65f, 0.0f));
    bunny->addMaterial(MaterialType::deferred, std::move(deferredMaterial));
    bunny->addMaterial(MaterialType::deferred_pbr, std::move(pbrMaterial));

    renderer.addModel(bunny);


    std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>();
    boxMesh->fromOBJ("assets/box.obj");

    std::unique_ptr<Material> boxMaterial = std::make_unique<Material>(
        glm::vec3(0.66, 0.66, 0.66),
        0.5f,
        64.0f
    );

    std::unique_ptr<Material> boxDeferredMaterial = std::make_unique<DeferredMaterial>(
        glm::vec3(0.66, 0.66, 0.66),
        0.1f,
        64.0f
    );

    std::unique_ptr<Material> boxPBRMaterial = std::make_unique<DeferredPBRMaterial>(
        glm::vec3(0.66, 0.66, 0.66),
        0.0f,
        0.0f
    );

    boxMaterial->setSide(Side::BACK);
    boxDeferredMaterial->setSide(Side::BACK);
    boxPBRMaterial->setSide(Side::BACK);

    std::shared_ptr<Model> box = std::make_shared<Model>(boxMesh, std::move(boxMaterial));
    box->addMaterial(MaterialType::deferred, std::move(boxDeferredMaterial));
    box->addMaterial(MaterialType::deferred_pbr, std::move(boxPBRMaterial));
    renderer.addModel(box);


    int lamp1Intensity = 2;
    Lamp lamp1(sphereMesh, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.8f, 0.6f, 0.4f), static_cast<float>(lamp1Intensity));
    lamp1.setScale(0.1f);

    renderer.addModel(lamp1.getModel());
    renderer.addLight(lamp1.getLight());

    Lamp lamp2(sphereMesh, glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.2f, 0.9f, 0.5f), 2.0f);
    lamp2.setScale(0.1f);

    renderer.addModel(lamp2.getModel());
    renderer.addLight(lamp2.getLight());

    Lamp lamp3(sphereMesh, glm::vec3(0.0f, 0.1f, 1.5f), glm::vec3(0.2f, 0.2f, 0.9f), 4.0f);
    lamp3.setScale(0.2f);

    renderer.addModel(lamp3.getModel());
    renderer.addLight(lamp3.getLight());

    Lamp lamp4(sphereMesh, glm::vec3(-1.0f, 1.5f, 1.5f), glm::vec3(0.9f, 0.2f, 0.1f), 4.0f);
    lamp4.setScale(0.05f);

    renderer.addModel(lamp4.getModel());
    renderer.addLight(lamp4.getLight());

    float frameLength = ONE_SECOND / FPS;
    auto last = std::chrono::steady_clock::now();
    bool quit = false;

    std::vector<float> exposureValues = {
        0.1f,
        0.2f,
        0.5f,
        1.0f,
        2.0f,
        5.0f,
        10.0f
    };

    lamp1.toggle();
    lamp2.toggle();
    lamp3.toggle();
    lamp4.toggle();

    sun->toggle();
    sun2->toggle();
    unsigned int currentExposure = 4;

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
                        renderer.toggleFXAA();
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
                    } else if (key == "S") {
                        renderer.toggleBlinnPhongShading();
                    } else if (key == "H") {
                        renderer.toggleHDR();
                    } else if (key == "G") {
                        renderer.toggleGammaCorrection();
                    } else if (key == "I") {
                        lamp1Intensity = (lamp1Intensity * 2) % 31;
                        lamp1.setIntensity(static_cast<float>(lamp1Intensity));
                    } else if (key == "B") {
                        renderer.toggleBloom();
                    } else if (key == "O") {
                        renderer.toggleSSAO();
                    } else if (key == "E") {
                        currentExposure++;
                        if (currentExposure >= exposureValues.size()) {
                            currentExposure = 0;
                        }

                        renderer.setExposure(exposureValues.at(currentExposure));
                    } else if (key == "P") {
                        renderer.togglePBR();
                    }
                }
            }

            if (quit) {
                break;
            }

            renderer.renderDeferred();
            last = now;
        }

    }
}
