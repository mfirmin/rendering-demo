#pragma once

#include "renderEffects/bloom.hpp"

#include <memory>
#include <SDL2/SDL.h>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

class Camera;
class Light;
class Model;

class DeferredTarget;
class RenderTarget;

class Renderer {
    public:
        Renderer(int width, int height, std::unique_ptr<Camera>&& camera);

        Renderer(Renderer&& other) = default;
        Renderer& operator=(Renderer&& other) = default;

        Renderer(const Renderer& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;

        void addModel(std::shared_ptr<Model> model);
        void addLight(std::shared_ptr<Light> light);

        void render();
        void renderDeferred();
        void toggleBloom();
        void toggleMSAA();
        void toggleBlinnPhongShading();
        void toggleHDR();
        void toggleGammaCorrection();
        void updateCameraRotation(glm::vec3 r);

        ~Renderer();
    private:
        SDL_Window* window = nullptr;
        SDL_Surface* screen = nullptr;

        int width;
        int height;

        std::unique_ptr<Camera> camera;

        std::vector<std::shared_ptr<Model>> models;

        std::vector<std::shared_ptr<Light>> lights;

        std::unique_ptr<RenderTarget> sceneTarget;
        std::unique_ptr<DeferredTarget> deferredTarget;

        struct {
            GLuint vertexArray = 0;
            GLuint vertexBuffer = 0;
            GLuint uvBuffer = 0;

            GLuint program;
        } screenObject;

        BloomEffect bloomEffect;

        bool MSAAEnabled = true;
        bool blinnPhongShadingEnabled = true;
        bool hdrEnabled = true;
        bool gammaCorrectionEnabled = true;
        bool bloomEnabled = true;

        bool initializeSDL();
        bool initializeGL();

        void initializeScreenObject();
};
