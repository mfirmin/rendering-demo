#include "renderer.hpp"

#include "camera.hpp"
#include "light/light.hpp"
#include "model.hpp"

#include <GL/glew.h>

#include <chrono>
#include <iostream>

constexpr GLuint GL_MAJOR = 3;
constexpr GLuint GL_MINOR = 3;

constexpr float ONE_SECOND = 1000.0f;
constexpr float FPS = 60.0f;

Renderer::Renderer(int width, int height, std::unique_ptr<Camera>&& camera) :
    width(width),
    height(height),
    camera(std::move(camera))
{
    std::cout << "Initializing SDL...\n";
    if (!initializeSDL()) {
        std::cout << "Failed to initialize SDL\n";
        return;
    }
    std::cout << "Initializing GL...\n";
    if (!initializeGL()) {
        std::cout << "Failed to initialize GL\n";
        return;
    }

    std::cout << "Ready\n";
}

Renderer::~Renderer() {
    SDL_DestroyWindow(window);

    window = nullptr;
    SDL_Quit();
}

bool Renderer::initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized\n";
    } else {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
        window = SDL_CreateWindow("Model Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            std::cout << "Window could not be created!\n";
        } else {
            screen = SDL_GetWindowSurface(window);
        }
    }
    return true;
}

bool Renderer::initializeGL() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_MINOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (context == nullptr) {
        std::cout << "Error creating openGL context: " << SDL_GetError() << "\n";
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        std::cout << "Error initializing GLEW\n";
        return false;
    }

    if (SDL_GL_SetSwapInterval(1) < 0)
    {
        std::cout << "Unable to set VSync\n";
        return false;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Enable writing to depth buffer
    glDepthMask(GL_TRUE);
    // Enable MultiSampling
    glEnable(GL_MULTISAMPLE);

    int value = 0;
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value);
    std::cout << "MultiSampleBuffers: " << value << "\n";
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value);
    std::cout << "MultiSampleSamples: " << value << "\n";

    return true;
}

void Renderer::addModel(Model&& model) {
    model.setProjectionAndViewMatrices(camera->getProjectionMatrix(), camera->getViewMatrix());
    model.setLights(lights);
    models.push_back(std::move(model));
}

void Renderer::addLight(std::unique_ptr<Light>&& light) {
    lights.push_back(std::move(light));

    for (auto& model : models) {
        model.setLights(lights);
    }
}

void Renderer::render() {
    // Bind the screen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& model : models) {
        model.draw();
    }

    // Bind back to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Swap
    SDL_GL_SwapWindow(window);
}

void Renderer::handleEvents(bool& quit) {
    SDL_Event e;

    while(SDL_PollEvent(&e) != 0) {
        if (
            e.type == SDL_QUIT ||
            (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
        ) {
            quit = true;
            return;
        }
    }
}

void Renderer::go() {
    float frameLength = ONE_SECOND / FPS;
    auto last = std::chrono::steady_clock::now();
    bool quit = false;
    while (!quit) {
        auto now = std::chrono::steady_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();

        if (dt >= frameLength) {
            handleEvents(quit);
            if (quit) {
                break;
            }
            render();
            last = now;
        }

    }
}
