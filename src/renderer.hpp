#pragma once

#include <SDL2/SDL.h>
#include <vector>

class Model;

class Renderer {
    public:
        Renderer(int width, int height);
        Renderer(Renderer&& other) = default;
        Renderer(const Renderer& other) = default;

        Renderer& operator=(const Renderer& other) = default;
        Renderer& operator=(Renderer&& other) = default;

        void addModel(Model&& model);
        void go();

        ~Renderer();
    private:
        SDL_Window* window = nullptr;
        SDL_Surface* screen = nullptr;

        int width;
        int height;

        std::vector<Model> models;

        bool initializeSDL();
        bool initializeGL();

        void handleEvents(bool& quit);

        void render();
};
