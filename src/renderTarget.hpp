#pragma once

#include <GL/glew.h>


class RenderTarget {
    public:
        RenderTarget(int width, int height);

        RenderTarget(RenderTarget&& other) = default;
        RenderTarget& operator=(RenderTarget&& other) = default;

        RenderTarget(const RenderTarget& other) = delete;
        RenderTarget& operator=(const RenderTarget& other) = delete;

        ~RenderTarget();

        GLuint getFramebuffer() {
            return buffer;
        }

        GLuint getTexture() {
            return texture;
        }

        GLuint getDepthBuffer() {
            return depthBuffer;
        }
    private:
        int width;
        int height;

        GLuint texture = 0;
        GLuint buffer = 0;
        GLuint depthBuffer = 0;
};
