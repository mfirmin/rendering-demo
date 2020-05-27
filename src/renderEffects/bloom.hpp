#pragma once

#include <array>
#include <GL/glew.h>

class BloomEffect {
    public:
        BloomEffect(int width, int height);

        BloomEffect(BloomEffect&& other) = default;
        BloomEffect& operator=(BloomEffect&& other) = default;

        BloomEffect(const BloomEffect& other) = delete;
        BloomEffect& operator=(const BloomEffect& other) = delete;

        ~BloomEffect();

        void initialize(GLuint st);

        void render(GLuint vao);

        void setSceneTexture(GLuint st) {
            sceneTexture = st;
        }

        GLuint getBrightnessTexture() {
            return brightnessTexture;
        }

        GLuint getBlurTexture() {
            // the last texture rendered into will be first texture in the array
            return blurTextures.at(0);
        }
    private:
        int width;
        int height;

        const int passes = 10;

        // input
        GLuint sceneTexture = 0;

        // output
        GLuint framebuffer = 0;
        GLuint brightnessTexture = 0;

        std::array<GLuint, 2> blurFBOs;
        std::array<GLuint, 2> blurTextures;

        GLuint brightnessProgram = 0;
        GLuint blurProgram = 0;

        void initializeBrightnessBuffers();
        void initializeBlurBuffers();
        void initializeBrightnessProgram();
        void initializeBlurProgram();
};
