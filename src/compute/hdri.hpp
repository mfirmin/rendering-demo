#pragma once

#include "mesh.hpp"

#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

class HDRI {
    public:
        HDRI();

        HDRI(HDRI&& other) = default;
        HDRI& operator=(HDRI&& other) = default;

        HDRI(const HDRI& other) = default;
        HDRI& operator=(const HDRI& other) = default;

        void initialize(std::string f);

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
        static constexpr unsigned int CUBE_FACES = 6;
        // Since diffuse irradiance doesn't vary much over N, we can store
        // a very low detailed texture
        static const unsigned int TEXTURE_WIDTH;
        static const unsigned int TEXTURE_HEIGHT;

        // lookAt(position, target, up)
        static const std::array<glm::mat4, CUBE_FACES> VIEW_MATRICES;

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

        Mesh cubeMesh;

        void loadTexture();
        void createCubemap();
        void createProgram();
        void renderToCubemap();
};
