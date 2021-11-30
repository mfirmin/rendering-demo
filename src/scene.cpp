#include "scene.hpp"

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
#include <memory>

Scene::Scene(int width, int height) : width(width), height(height) {}

// TODO (mfirmin): Read the scene data (lights, camera, models, etc) from an input
// text file rather than hard coding it
void Scene::initialize() {
    // 1. Initialize the Camera and Renderer
    auto aspect = static_cast<float>(width) / static_cast<float>(height);

    auto camera = std::make_unique<Camera>(aspect, 45.0f, -8.000f, glm::vec3(0.0f, 0.0f, 0.0f));
    renderer = std::make_unique<Renderer>(width, height, std::move(camera));
    renderer->setEnvironmentMap("assets/images/grand_canyon.hdr");

    // 2. Initialize the scene lights (point and directional
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

    renderer->addLight(sun);
    renderer->addLight(sun2);

    sun->toggle();
    sun2->toggle();

    // store the lights for manipulation later
    directionalLights = {
        sun,
        sun2
    };

    // Note that lamps are combined light + model, so we need to pass in a mesh for these
    std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>();
    sphereMesh->fromOBJ("assets/sphere.obj");

    createLamp(sphereMesh, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.8f, 0.6f, 0.4f), static_cast<float>(lamp1Intensity), 0.1f);

    createLamp(sphereMesh, glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.2f, 0.9f, 0.5f), 2.0f, 0.1f);

    createLamp(sphereMesh, glm::vec3(0.0f, 0.1f, 1.5f), glm::vec3(0.2f, 0.2f, 0.9f), 4.0f, 0.2f);

    createLamp(sphereMesh, glm::vec3(-1.0f, 1.5f, 1.5f), glm::vec3(0.9f, 0.2f, 0.1f), 4.0f, 0.05f);

    // 3. Create the central model
    std::shared_ptr<Mesh> bunnyMesh = std::make_shared<Mesh>();
    bunnyMesh->fromOBJ("assets/bunny.obj");

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

    std::shared_ptr<Model> bunny = std::make_shared<Model>(bunnyMesh, std::move(material));
    bunny->setPosition(glm::vec3(0.3f, -1.65f, 0.0f));
    bunny->addMaterial(MaterialType::deferred, std::move(deferredMaterial));
    bunny->addMaterial(MaterialType::deferred_pbr, std::move(pbrMaterial));

    renderer->addModel(bunny);

    model = bunny;
}

void Scene::createLamp(
    std::shared_ptr<Mesh> mesh,
    glm::vec3 position,
    glm::vec3 color,
    float intensity,
    float scale
) {
    Lamp lamp(mesh, position, color, intensity);
    lamp.setScale(scale);

    renderer->addModel(lamp.getModel());
    renderer->addLight(lamp.getLight());
    lamp.toggle();

    lamps.push_back(lamp);
}

void Scene::go() {
    bool quit = false;
    bool mouseDown = false;

    const float frameLength = ONE_SECOND / FPS;

    auto last = std::chrono::steady_clock::now();

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
                        renderer->updateCameraRotation(glm::vec3(-static_cast<float>(y) / 100.0f, -static_cast<float>(x) / 100.0f, 0.0f));
                    } else {
                        SDL_GetRelativeMouseState(&x, &y);
                    }
                } else if (e.type == SDL_MOUSEBUTTONUP) {
                    mouseDown = false;
                } else if (e.type == SDL_KEYUP) {
                    auto key = std::string(SDL_GetKeyName(e.key.keysym.sym));
                    if (key == "A") {
                        renderer->toggleMSAA();
                        renderer->toggleFXAA();
                    } else if (key == "1") {
                        lamps.at(0).toggle();
                    } else if (key == "2") {
                        lamps.at(1).toggle();
                    } else if (key == "3") {
                        lamps.at(2).toggle();
                    } else if (key == "4") {
                        lamps.at(3).toggle();
                    } else if (key == "L") {
                        // Primary Lighting
                        for (auto& dl : directionalLights) {
                            dl->toggle();
                        }
                    } else if (key == "S") {
                        renderer->toggleBlinnPhongShading();
                    } else if (key == "H") {
                        renderer->toggleHDR();
                    } else if (key == "G") {
                        renderer->toggleGammaCorrection();
                    } else if (key == "I") {
                        lamp1Intensity = (lamp1Intensity * 2) % 31;
                        lamps.at(0).setIntensity(static_cast<float>(lamp1Intensity));
                    } else if (key == "B") {
                        renderer->toggleBloom();
                    } else if (key == "O") {
                        renderer->toggleSSAO();
                    } else if (key == "E") {
                        exposure++;
                        if (exposure >= EXPOSURE_VALUES.size()) {
                            exposure = 0;
                        }

                        renderer->setExposure(EXPOSURE_VALUES.at(exposure));
                    } else if (key == "P") {
                        renderer->togglePBR();
                    } else if (key == "M") {
                        if (pbrMaterialType == PBRPreset::metallic) {
                            pbrMaterialType = PBRPreset::glossy;
                            model->setRoughness(0.0);
                            model->setMetalness(0.0);
                        } else if (pbrMaterialType == PBRPreset::glossy){
                            pbrMaterialType = PBRPreset::rough;
                            model->setRoughness(1.0);
                            model->setMetalness(0.0);
                        } else if (pbrMaterialType == PBRPreset::rough) {
                            pbrMaterialType = PBRPreset::rough_metal;
                            model->setRoughness(0.6);
                            model->setMetalness(1.0);
                        } else if (pbrMaterialType == PBRPreset::rough_metal) {
                            pbrMaterialType = PBRPreset::metallic;
                            model->setRoughness(0.2);
                            model->setMetalness(1.0);
                        }
                    } else if (key == "Z") {
                        renderer->toggleIBL();
                    }
                }
            }

            if (quit) {
                break;
            }

            // renderer.renderIBLTest(hdri);
            // renderer.render();
            renderer->renderDeferred();
            last = now;
        }

    }
}

const std::vector<float> Scene::EXPOSURE_VALUES = {
    0.1f,
    0.2f,
    0.5f,
    1.0f,
    2.0f,
    5.0f,
    10.0f
};

const float Scene::ONE_SECOND = 1000.0f;
const float Scene::FPS = 60.0f;
