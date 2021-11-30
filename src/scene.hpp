#pragma once

#include "renderer.hpp"

#include "lamp.hpp"

class DirectionalLight;

// Scene initializes all scene elements (camera, renderer, etc)
// And begins the render/event loop
class Scene {
    public:
        Scene(int width, int height);

        Scene(Scene&& other) = default;
        Scene& operator=(Scene&& other) = default;

        Scene(const Scene& other) = delete;
        Scene& operator=(const Scene& other) = delete;

        ~Scene() = default;

        void initialize();

        // Start the main loop
        void go();
    private:
        enum class PBRPreset {
            metallic, // metal = 1, roughness = 0
            glossy, // metal = 0, roughness = 0
            rough, // metal = 0, roughness = 1
            rough_metal // metal = 1, roughness = 1
        };

        static const std::vector<float> exposureValues;
        static const float ONE_SECOND;
        static const float FPS;

        int width;
        int height;

        std::unique_ptr<Renderer> renderer = nullptr;
        std::vector<Lamp> lamps;
        std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
        std::shared_ptr<Model> model;

        unsigned int exposure = 3;
        int lamp1Intensity = 2;

        PBRPreset pbrMaterialType = PBRPreset::metallic;

        void createLamp(
            std::shared_ptr<Mesh> mesh,
            glm::vec3 position,
            glm::vec3 color,
            float intensity,
            float scale
        );
};
