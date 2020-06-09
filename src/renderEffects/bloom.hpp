#pragma once

#include <array>
#include <GL/glew.h>

class BloomEffect {
    public:
        BloomEffect(int width, int height);

        BloomEffect(BloomEffect&& other) = default;
        BloomEffect& operator=(BloomEffect&& other) = default;

        BloomEffect(const BloomEffect& other) = default;
        BloomEffect& operator=(const BloomEffect& other) = default;

        ~BloomEffect();

        void initialize();

        void render(GLuint vao, GLuint sceneTexture);

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

        static const int passes = 10;

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
