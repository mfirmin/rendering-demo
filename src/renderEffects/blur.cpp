#include "blur.hpp"

#include "gl/shaderUtils.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>

BlurEffect::BlurEffect(int w, int h) :
    width(w),
    height(h)
{}

BlurEffect::~BlurEffect() {
    // TODO: Free buffers
}

// Must call this AFTER GL/SDL have been initialized
void BlurEffect::initialize() {
    createGLObjects();
    createProgram();
}

void BlurEffect::createGLObjects() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    // just need red component (greyscale)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error creating Blur: Error creating framebuffer\n";
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void BlurEffect::createProgram() {
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

        uniform sampler2D input;

        in vec2 vUv;

        out float fragColor;

        void main() {
            vec2 texelSize = 1.0 / vec2(textureSize(input, 0));
            float result = 0.0;
            for (int i = -2; i < 2; i++) {
                for (int j = -2; j < 2; j++) {
                    vec2 offset = vec2(float(i), float(j)) * texelSize;
                    result += texture(input, vUv + offset).r;
                }
            }

            fragColor = result / (4.0 * 4.0);
        }
    )";

    program = ShaderUtils::compile(vertexShader, fragmentShader);
}


void BlurEffect::render(GLuint vao, GLuint input) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // no depth buffer, so no need to clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    // bind the position texture to texture slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input);
    glUniform1i(glGetUniformLocation(program, "input"), 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
