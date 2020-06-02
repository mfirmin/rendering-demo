#pragma once

#include "blur.hpp"

#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class SSAOEffect {
    public:
        SSAOEffect(int width, int height);

        SSAOEffect(SSAOEffect&& other) = default;
        SSAOEffect& operator=(SSAOEffect&& other) = default;

        SSAOEffect(const SSAOEffect& other) = default;
        SSAOEffect& operator=(const SSAOEffect& other) = default;

        ~SSAOEffect();

        void initialize();

        void render(GLuint vao, GLuint gPosition, GLuint gNormal);

        void renderDebug(GLuint vao);

        void setProjectionMatrix(
            const glm::mat4& projectionMatrix
        );

        GLuint getFramebuffer() {
            return fbo;
        }

        GLuint getRawAmbientOcculsionTexture() {
            return ambientOcclusionTexture;
        }

        GLuint getAmbientOcculsionTexture() {
            return blurEffect.getResult();
        }
    private:
        int width;
        int height;

        GLuint fbo = 0;
        GLuint ambientOcclusionTexture = 0;

        BlurEffect blurEffect;

        std::vector<glm::vec3> kernel = {};

        GLuint kernelNoiseTexture = 0;

        GLuint program = 0;
        GLuint debugProgram = 0;

        void createGLObjects();
        void constructKernel();
        void constructKernelNoise();
        void createProgram();
        void createDebugProgram();
};
