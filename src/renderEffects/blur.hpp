#pragma once

#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

// *Simple* blur effect
class BlurEffect {
    public:
        BlurEffect(int width, int height);

        BlurEffect(BlurEffect&& other) = default;
        BlurEffect& operator=(BlurEffect&& other) = default;

        BlurEffect(const BlurEffect& other) = default;
        BlurEffect& operator=(const BlurEffect& other) = default;

        ~BlurEffect();

        void initialize();

        void render(GLuint vao, GLuint input);

        GLuint getResult() {
            return result;
        }
    private:
        int width;
        int height;

        GLuint fbo = 0;
        GLuint result = 0;

        GLuint program = 0;

        void createGLObjects();
        void createProgram();
};
