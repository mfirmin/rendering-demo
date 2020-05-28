#pragma once

#include <GL/glew.h>


class DeferredTarget {
    public:
        DeferredTarget(int width, int height);

        DeferredTarget(DeferredTarget&& other) = default;
        DeferredTarget& operator=(DeferredTarget&& other) = default;

        DeferredTarget(const DeferredTarget& other) = delete;
        DeferredTarget& operator=(const DeferredTarget& other) = delete;

        ~DeferredTarget();

        GLuint getFramebuffer() {
            return fbo;
        }

        GLuint getPosition() {
            return positionTexture;
        }

        GLuint getNormal() {
            return normalTexture;
        }

        GLuint getAlbedo() {
            return albedoTexture;
        }
    private:
        int width;
        int height;

        GLuint fbo = 0;
        GLuint positionTexture = 0;
        GLuint normalTexture = 0;
        GLuint albedoTexture = 0;
};
