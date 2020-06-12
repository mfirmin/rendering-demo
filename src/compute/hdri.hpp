#pragma once

#include "mesh.hpp"

#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

// create an image for each of the 6 faces of the cubemap
const unsigned int CUBE_FACES = 6;
const unsigned int TEXTURE_WIDTH = 4096;
const unsigned int TEXTURE_HEIGHT = TEXTURE_WIDTH;

const glm::vec3 ZERO(0.0f, 0.0f, 0.0f);
const glm::vec3 LEFT(-1.0f, 0.0f, 0.0f);
const glm::vec3 RIGHT(1.0f, 0.0f, 0.0f);
const glm::vec3 UP(0.0f, 1.0f, 0.0f);
const glm::vec3 DOWN(0.0f, -1.0f, 0.0f);
const glm::vec3 FORWARD(0.0f, 0.0f, 1.0f);
const glm::vec3 BACKWARD(0.0f, 0.0f, -1.0f);

class HDRI {
    public:
        HDRI(std::string filename);

        HDRI(HDRI&& other) = default;
        HDRI& operator=(HDRI&& other) = default;

        HDRI(const HDRI& other) = default;
        HDRI& operator=(const HDRI& other) = default;

        void initialize();

        void setProjectionAndViewMatrices(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);

        GLuint getTexture() {
            return texture;
        }

        GLuint getCubemap() {
            return cubemapTexture;
        }

        void renderCube();

        ~HDRI();
    private:
        std::string filename;

        int width = 0;
        int height = 0;

        int numChannels = 0;

        GLuint texture = 0;

        GLuint cubemapTexture = 0;
        GLuint cubemapProgram = 0;

        // fbo used in the process of creating the cubemap
        GLuint fbo = 0;
        GLuint depthBuffer = 0;

        glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        // lookAt(position, target, up)
        std::array<glm::mat4, CUBE_FACES> viewMatrices = {
            glm::lookAt(ZERO, RIGHT, DOWN),
            glm::lookAt(ZERO, LEFT, DOWN),
            glm::lookAt(ZERO, UP, FORWARD),
            glm::lookAt(ZERO, DOWN, BACKWARD),
            glm::lookAt(ZERO, FORWARD, DOWN),
            glm::lookAt(ZERO, BACKWARD, DOWN)
        };

        Mesh cubeMesh;

        void loadTexture();
        void createCubemap();
        void createProgram();
        void renderToCubemap();
};
