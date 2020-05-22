#pragma once

#include <memory>
#include <SDL2/SDL.h>
#include <vector>

#include <GL/glew.h>

class Camera;
class Light;
class Model;
class RenderTarget;

class Renderer {
    public:
        Renderer(int width, int height, std::unique_ptr<Camera>&& camera);

        Renderer(Renderer&& other) = default;
        Renderer& operator=(Renderer&& other) = default;

        Renderer(const Renderer& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;

        void addModel(Model&& model);
        void addLight(std::unique_ptr<Light>&& light);

        void go();

        ~Renderer();
    private:
        SDL_Window* window = nullptr;
        SDL_Surface* screen = nullptr;

        int width;
        int height;

        std::unique_ptr<Camera> camera;

        std::vector<Model> models;

        std::vector<std::unique_ptr<Light>> lights;

        std::unique_ptr<RenderTarget> sceneTarget;

        struct {
            GLuint vertexArray = 0;
            GLuint vertexBuffer = 0;
            GLuint uvBuffer = 0;

            GLuint program;
        } screenObject;

        bool initializeSDL();
        bool initializeGL();

        void initializeScreenObject();

        void handleEvents(bool& quit);

        void render();
};
