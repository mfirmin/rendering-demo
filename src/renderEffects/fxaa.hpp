#pragma once

#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class FXAAEffect {
    public:
        FXAAEffect(int width, int height);

        FXAAEffect(FXAAEffect&& other) = default;
        FXAAEffect& operator=(FXAAEffect&& other) = default;

        FXAAEffect(const FXAAEffect& other) = default;
        FXAAEffect& operator=(const FXAAEffect& other) = default;

        ~FXAAEffect();

        void initialize();

        void render(GLuint vao, GLuint input);
    private:
        int width;
        int height;

        GLuint program = 0;

        void createProgram();
};
