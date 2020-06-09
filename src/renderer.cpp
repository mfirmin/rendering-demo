#include "renderer.hpp"

#include "camera.hpp"
#include "gl/shaderUtils.hpp"
#include "light/light.hpp"
#include "material/material.hpp"
#include "model.hpp"
#include "renderTarget.hpp"

#include <GL/glew.h>

#include <iostream>

constexpr GLuint GL_MAJOR = 3;
constexpr GLuint GL_MINOR = 3;

Renderer::Renderer(int width, int height, std::unique_ptr<Camera>&& camera) :
    width(width),
    height(height),
    camera(std::move(camera)),
    bloomEffect(width, height),
    deferredShadingEffect(width, height),
    deferredPBREffect(width, height),
    ssaoEffect(width, height),
    fxaaEffect(width, height)
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

    initializeScreenObject();

    sceneTarget = std::make_unique<RenderTarget>(width, height);

    deferredShadingEffect.initialize();
    deferredPBREffect.initialize();
    ssaoEffect.initialize();
    bloomEffect.initialize();
    fxaaEffect.initialize();

    // composits bloom, hdr, and gammaCorrection
    // Final step before passing to fxaa
    initializeCompositingPass();
    initializeBaseProgram();

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
    if (glewError != GLEW_OK) {
        std::cout << "Error initializing GLEW\n";
        return false;
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
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
    // Enable face culling
    glEnable(GL_CULL_FACE);

    return true;
}

void Renderer::initializeScreenObject() {
    glGenVertexArrays(1, &screenObject.vertexArray);
    glGenBuffers(1, &screenObject.vertexBuffer);
    glGenBuffers(1, &screenObject.uvBuffer);

    glBindVertexArray(screenObject.vertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, screenObject.vertexBuffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // NDC Coords
    std::vector<float> vertices = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, screenObject.uvBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    std::vector<float> uvs = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GL_FLOAT), uvs.data(), GL_STATIC_DRAW);
}

void Renderer::initializeCompositingPass() {
    glGenFramebuffers(1, &compositingPass.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, compositingPass.fbo);

    glGenTextures(1, &compositingPass.result);
    glBindTexture(GL_TEXTURE_2D, compositingPass.result);
    // just need red component (greyscale)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compositingPass.result, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error creating FXAA: Error creating framebuffer\n";
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Create Compositing Shader
    std::string vertexShaderSource = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";


    std::string fragmentShaderSource = R"(
        #version 330

        // scene is a floating point (HDR) texture
        uniform sampler2D scene;
        uniform sampler2D bloomBlur;

        uniform float hdrEnabled;
        uniform float gammaCorrectionEnabled;
        uniform float bloomEnabled;

        uniform float exposure;

        in vec2 vUv;

        out vec4 fragColor;

        void main() {
            const float gamma = 2.2;

            vec3 color = texture(scene, vUv).rgb;

            if (bloomEnabled > 0.5) {
                color += texture(bloomBlur, vUv).rgb;
            }

            // Reinhard Tone Mapping
            if (hdrEnabled > 0.5) {
                // color = color / (color + vec3(1.0));
                color = vec3(1.0) - exp(-color * exposure);
            }

            // Gamma correction
            if (gammaCorrectionEnabled > 0.5) {
                color = pow(color, vec3(1.0 / gamma));
            }

            fragColor = vec4(color, 1.0);
        }
    )";

    compositingPass.program = ShaderUtils::compile(vertexShaderSource, fragmentShaderSource);

    glUseProgram(compositingPass.program);
    glUniform1f(glGetUniformLocation(compositingPass.program, "hdrEnabled"), 1.0f);
    glUniform1f(glGetUniformLocation(compositingPass.program, "gammaCorrectionEnabled"), 1.0f);
    glUniform1f(glGetUniformLocation(compositingPass.program, "bloomEnabled"), 1.0f);
    glUniform1f(glGetUniformLocation(compositingPass.program, "exposure"), 1.0f);
    glUseProgram(0);
}

void Renderer::initializeBaseProgram() {
    // Create Basic Shader (render texture to screen)
    std::string vertexShaderSource = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";


    std::string fragmentShaderSource = R"(
        #version 330

        uniform sampler2D scene;

        in vec2 vUv;

        out vec4 fragColor;

        void main() {
            vec3 color = texture(scene, vUv).rgb;
            fragColor = vec4(color, 1.0);
        }
    )";

    baseProgram = ShaderUtils::compile(vertexShaderSource, fragmentShaderSource);
}


void Renderer::addModel(std::shared_ptr<Model> model) {
    model->setProjectionAndViewMatrices(camera->getProjectionMatrix(), camera->getViewMatrix());
    model->setLights(lights);
    models.push_back(model);
}

void Renderer::addLight(std::shared_ptr<Light> light) {
    lights.push_back(light);

    for (auto& model : models) {
        model->setLights(lights);
    }
}

void Renderer::updateCameraRotation(glm::vec3 r) {
    camera->addRotation(r);
}

void Renderer::toggleBloom() {
    bloomEnabled = !bloomEnabled;
    glUseProgram(compositingPass.program);
    glUniform1f(glGetUniformLocation(compositingPass.program, "bloomEnabled"), bloomEnabled ? 1.0f : 0.0f);
    glUseProgram(0);
}

void Renderer::toggleGammaCorrection() {
    gammaCorrectionEnabled = !gammaCorrectionEnabled;
    glUseProgram(compositingPass.program);
    glUniform1f(glGetUniformLocation(compositingPass.program, "gammaCorrectionEnabled"), gammaCorrectionEnabled ? 1.0f : 0.0f);
    glUseProgram(0);
}

void Renderer::toggleHDR() {
    hdrEnabled = !hdrEnabled;
    glUseProgram(compositingPass.program);
    glUniform1f(glGetUniformLocation(compositingPass.program, "hdrEnabled"), hdrEnabled ? 1.0f : 0.0f);
    glUseProgram(0);
}

void Renderer::toggleMSAA() {
    if (MSAAEnabled) {
        glDisable(GL_MULTISAMPLE);
    } else {
        glEnable(GL_MULTISAMPLE);
    }
    MSAAEnabled = !MSAAEnabled;
}

void Renderer::toggleFXAA() {
    FXAAEnabled = !FXAAEnabled;
}

void Renderer::togglePBR() {
    pbrEnabled = !pbrEnabled;
}

void Renderer::toggleBlinnPhongShading() {
    blinnPhongShadingEnabled = !blinnPhongShadingEnabled;
    for (auto model : models) {
        model->toggleBlinnPhongShading(blinnPhongShadingEnabled);
    }
    deferredShadingEffect.toggleBlinnPhongShading(blinnPhongShadingEnabled);
}

void Renderer::toggleSSAO() {
    ssaoEnabled = !ssaoEnabled;
    deferredShadingEffect.toggleSSAO(ssaoEnabled);
    deferredPBREffect.toggleSSAO(ssaoEnabled);
}

void Renderer::setExposure(float value) {
    glUseProgram(compositingPass.program);
    glUniform1f(glGetUniformLocation(compositingPass.program, "exposure"), value);
    glUseProgram(0);
}

void Renderer::render() {
    auto msFBO = sceneTarget->getMultiSampleFramebuffer();
    auto outFBO = sceneTarget->getOutputFramebuffer();
    // Bind the scene buffer
    glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (camera->isDirty()) {
        for (auto& model : models) {
            model->setProjectionAndViewMatrices(camera->getProjectionMatrix(), camera->getViewMatrix());
        }
        camera->setDirty(false);
    }
    // TODO(mfirmin): Check if lights are dirty
    for (auto& model : models) {
        model->setLights(lights);
    }

    for (auto& model : models) {
        model->applyModelMatrix();
        model->draw(MaterialType::standard);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, outFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // render the bloom effect
    bloomEffect.render(screenObject.vertexArray, sceneTarget->getTexture());

    // Bind the screen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the screen object to it (using the scene render target)
    glBindVertexArray(screenObject.vertexArray);
    glUseProgram(compositingPass.program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTarget->getTexture());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomEffect.getBlurTexture());

    glUniform1i(glGetUniformLocation(compositingPass.program, "scene"), 0);
    glUniform1i(glGetUniformLocation(compositingPass.program, "bloomBlur"), 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);

    // Swap
    SDL_GL_SwapWindow(window);
}

void Renderer::renderDeferred() {
    GLuint deferredBuffer = 0;
    if (pbrEnabled) {
        deferredBuffer = deferredPBREffect.getFramebuffer();
    } else {
        deferredBuffer = deferredShadingEffect.getFramebuffer();
    }
    // Bind the scene buffer
    glBindFramebuffer(GL_FRAMEBUFFER, deferredBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (camera->isDirty()) {
        for (auto& model : models) {
            model->setProjectionAndViewMatrices(camera->getProjectionMatrix(), camera->getViewMatrix());
        }

        deferredPBREffect.setViewMatrix(camera->getViewMatrix());
        deferredShadingEffect.setViewMatrix(camera->getViewMatrix());

        ssaoEffect.setProjectionMatrix(camera->getProjectionMatrix());
        camera->setDirty(false);
    }
    // TODO(mfirmin): Check if lights are dirty
    for (auto& model : models) {
        model->setLights(lights);
    }

    if (pbrEnabled) {
        deferredPBREffect.setLights(lights);
    } else {
        deferredShadingEffect.setLights(lights);
    }

    // ensure models have deferred material applied
    // todo: support multiple materials per model
    for (auto& model : models) {
        model->applyModelMatrix();
        model->draw(pbrEnabled ? MaterialType::deferred_pbr : MaterialType::deferred);
    }

    // render the ambient occlusion term

    if (pbrEnabled) {
        ssaoEffect.render(screenObject.vertexArray, deferredPBREffect.getPosition(), deferredPBREffect.getNormal());
        bloomEffect.render(screenObject.vertexArray, deferredPBREffect.getOutputTexture());
        // do the deferred lighting step
        deferredPBREffect.render(screenObject.vertexArray, ssaoEffect.getAmbientOcculsionTexture());
    } else {
        ssaoEffect.render(screenObject.vertexArray, deferredShadingEffect.getPosition(), deferredShadingEffect.getNormal());
        bloomEffect.render(screenObject.vertexArray, deferredShadingEffect.getOutputTexture());
        // do the deferred lighting step
        deferredShadingEffect.render(screenObject.vertexArray, ssaoEffect.getAmbientOcculsionTexture());
    }

    // Render the compositing pass
    // Bind the screen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, compositingPass.fbo);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the screen object to it (using the scene render target)
    glBindVertexArray(screenObject.vertexArray);
    glUseProgram(compositingPass.program);

    glActiveTexture(GL_TEXTURE0);
    if (pbrEnabled) {
        glBindTexture(GL_TEXTURE_2D, deferredPBREffect.getOutputTexture());
    } else {
        glBindTexture(GL_TEXTURE_2D, deferredShadingEffect.getOutputTexture());
    }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomEffect.getBlurTexture());

    glUniform1i(glGetUniformLocation(compositingPass.program, "scene"), 0);
    glUniform1i(glGetUniformLocation(compositingPass.program, "bloomBlur"), 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);


    if (FXAAEnabled) {
        // perform anti-aliasing pass (fxaa) which is rendered to the screen
        fxaaEffect.render(screenObject.vertexArray, compositingPass.result);
    } else {
        // render scene to screen without aa
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(screenObject.vertexArray);
        glUseProgram(baseProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, compositingPass.result);
        glUniform1i(glGetUniformLocation(baseProgram, "scene"), 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glUseProgram(0);
    }

    // Swap
    SDL_GL_SwapWindow(window);
}
