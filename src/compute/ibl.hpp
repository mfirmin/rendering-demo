#pragma once

#include "mesh.hpp"

#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>


class IBL {
    public:
        IBL();

        IBL(IBL&& other) = default;
        IBL& operator=(IBL&& other) = default;

        IBL(const IBL& other) = default;
        IBL& operator=(const IBL& other) = default;

        void initialize(GLuint em);

        GLuint getDiffuseIrradiance() {
            return diffuseIrradianceTexture;
        }

        void setEnvironmentMap(GLuint em);

        ~IBL();
    private:
        static constexpr unsigned int CUBE_FACES = 6;
        // Since diffuse irradiance doesn't vary much over N, we can store
        // a very low detailed texture
        static constexpr unsigned int TEXTURE_WIDTH = 32;
        static constexpr unsigned int TEXTURE_HEIGHT = TEXTURE_WIDTH;

        static constexpr glm::vec3 ZERO = glm::vec3(0.0f, 0.0f, 0.0f);
        static constexpr glm::vec3 LEFT = glm::vec3(-1.0f, 0.0f, 0.0f);
        static constexpr glm::vec3 RIGHT= glm::vec3(1.0f, 0.0f, 0.0f);
        static constexpr glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);
        static constexpr glm::vec3 DOWN = glm::vec3(0.0f, -1.0f, 0.0f);
        static constexpr glm::vec3 FORWARD = glm::vec3(0.0f, 0.0f, 1.0f);
        static constexpr glm::vec3 BACKWARD = glm::vec3(0.0f, 0.0f, -1.0f);

        int width = 0;
        int height = 0;

        GLuint environmentMap = 0;
        GLuint diffuseIrradianceTexture = 0;
        GLuint diffuseIrradianceProgram = 0;

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

        void createTexture();
        void createProgram();
        // Input: Environment cubemap texture
        // Output: Sets the diffuseIrradianceTexture to the convolved environmentMap
        void renderToDiffuseIrradiance();
};
