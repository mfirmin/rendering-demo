#include "bloom.hpp"

#include "gl/shaderUtils.hpp"

#include <array>
#include <iostream>

BloomEffect::BloomEffect(int w, int h) :
    width(w),
    height(h)
{}

BloomEffect::~BloomEffect() {
    // TODO: Free buffers
}

// Must call this AFTER GL/SDL have been initialized
void BloomEffect::initialize(GLuint st) {
    initializeBrightnessBuffers();
    initializeBlurBuffers();
    initializeBrightnessProgram();
    initializeBlurProgram();
    setSceneTexture(st);
}

void BloomEffect::initializeBrightnessBuffers() {
    // initialize the framebuffer for the brightness texture
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Initialize the texture for the brightness buffer
    glGenTextures(1, &brightnessTexture);
    glBindTexture(GL_TEXTURE_2D, brightnessTexture);

    // floating point texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightnessTexture, 0);

    std::array<GLenum, 1> drawbuffers = { GL_COLOR_ATTACHMENT0 };

    glDrawBuffers(1, drawbuffers.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error creating BloomEffect: Error creating multisample framebuffer\n";
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomEffect::initializeBlurBuffers() {
    // initialize the framebuffer for the brightness texture
    glGenFramebuffers(2, blurFBOs.data());
    // Initialize the texture for the brightness buffer
    glGenTextures(2, blurTextures.data());

    std::array<GLenum, 1> drawbuffers = { GL_COLOR_ATTACHMENT0 };

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs.at(i));
        glBindTexture(GL_TEXTURE_2D, blurTextures.at(i));

        // textures should be floating point
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTextures.at(i), 0);

        glDrawBuffers(1, drawbuffers.data());

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Error creating BloomEffect: Error creating multisample framebuffer\n";
            return;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomEffect::initializeBrightnessProgram() {
    std::string vertexShader = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    std::string fragmentShader = R"(
        #version 330

        // scene is an hdr, floating point texture
        uniform sampler2D scene;

        in vec2 vUv;

        out vec4 fragColor;

        void main() {
            vec3 color = texture(scene, vUv).rgb;

            // convert to greyscale
            float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

            // ignore areas which are not brighter than 1.0
            if (brightness <= 1.0) {
                color = vec3(0.0, 0.0, 0.0);
            }

            fragColor = vec4(color, 1.0);
        }
    )";

    brightnessProgram = ShaderUtils::compile(vertexShader, fragmentShader);
}

void BloomEffect::initializeBlurProgram() {
    std::string vertexShader = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    std::string fragmentShader = R"(
        #version 330

        const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

        // scene is an hdr, floating point texture
        uniform sampler2D input;
        uniform float horizontal;

        in vec2 vUv;

        out vec4 fragColor;

        void main() {
            vec2 pixelSize = 1.0 / textureSize(input, 0);
            vec3 color = texture(input, vUv).rgb * weight[0];

            vec2 offset = horizontal > 0.5 ? vec2(pixelSize.x, 0.0) : vec2(0.0, pixelSize.y);

            for (int i = 1; i < 5; i++) {
                color += texture(input, vUv + i * offset).rgb * weight[i];
                color += texture(input, vUv - i * offset).rgb * weight[i];
            }

            fragColor = vec4(color, 1.0);
        }
    )";

    blurProgram = ShaderUtils::compile(vertexShader, fragmentShader);

    glUseProgram(blurProgram);
    glUniform1f(glGetUniformLocation(blurProgram, "horizontal"), 1.0);
    glUniform1i(glGetUniformLocation(blurProgram, "input"), 0);
    glUseProgram(0);
}

// vao should be a triangle strip quad
void BloomEffect::render(GLuint vao) {
    // bind the framebuffer for the bloom effect
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // no depth buffer, so no need to clear it
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(brightnessProgram);

    // bind the scene texture to texture slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glUniform1i(glGetUniformLocation(brightnessProgram, "scene"), 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);


    // At this point, the brightnessTexture attached to the framebuffer should contain
    // only the bright points of the scene
    // Now we need to blur it.


    int horizontal = 1;
    bool first = true;
    glUseProgram(blurProgram);
    for (int i = 0; i < passes; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs.at(horizontal));
        glUniform1f(glGetUniformLocation(blurProgram, "horizontal"), static_cast<float>(horizontal));

        auto input = horizontal == 1 ? 0 : 1;
        glBindTexture(GL_TEXTURE_2D, first ? brightnessTexture : blurTextures.at(input));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        horizontal = horizontal == 1 ? 0 : 1;

        first = false;
    }

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
