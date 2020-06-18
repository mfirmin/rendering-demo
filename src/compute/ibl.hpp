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

        void initialize(GLuint em, GLuint vao);

        GLuint getDiffuseIrradiance() {
            return diffuseIrradianceMap;
        }

        GLuint getPrefilteredMap() {
            return prefilterMap;
        }

        GLuint getIntegratedBRDFMap() {
            return integratedBRDFMap;
        }

        void setEnvironmentMap(GLuint em);

        ~IBL();
    private:
        static constexpr unsigned int CUBE_FACES = 6;
        // Since diffuse irradiance doesn't vary much over N, we can store
        // a very low detailed texture
        static constexpr unsigned int DIFFUSE_IRRADIANCE_TEXTURE_WIDTH = 32;
        static constexpr unsigned int DIFFUSE_IRRADIANCE_TEXTURE_HEIGHT = DIFFUSE_IRRADIANCE_TEXTURE_WIDTH;

        static constexpr unsigned int PREFILTERED_TEXTURE_MIPMAP_LEVELS = 5;
        static constexpr unsigned int PREFILTERED_TEXTURE_WIDTH = 128;
        static constexpr unsigned int PREFILTERED_TEXTURE_HEIGHT = PREFILTERED_TEXTURE_WIDTH;

        static constexpr unsigned int INTEGRATED_BRDF_TEXTURE_WIDTH = 512;
        static constexpr unsigned int INTEGRATED_BRDF_TEXTURE_HEIGHT = INTEGRATED_BRDF_TEXTURE_WIDTH;

        static constexpr glm::vec3 ZERO = glm::vec3(0.0f, 0.0f, 0.0f);
        static constexpr glm::vec3 LEFT = glm::vec3(-1.0f, 0.0f, 0.0f);
        static constexpr glm::vec3 RIGHT= glm::vec3(1.0f, 0.0f, 0.0f);
        static constexpr glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);
        static constexpr glm::vec3 DOWN = glm::vec3(0.0f, -1.0f, 0.0f);
        static constexpr glm::vec3 FORWARD = glm::vec3(0.0f, 0.0f, 1.0f);
        static constexpr glm::vec3 BACKWARD = glm::vec3(0.0f, 0.0f, -1.0f);

        int width = 0;
        int height = 0;

        GLuint screenVertexArray = 0;

        GLuint environmentMap = 0;
        GLuint diffuseIrradianceMap = 0;
        GLuint diffuseIrradianceProgram = 0;

        // Prefiltered environment map for the specular term
        GLuint prefilterMap = 0;
        GLuint prefilterProgram = 0;

        // IntegratedBRDF Map
        GLuint integratedBRDFMap = 0;
        GLuint integrateBRDFProgram = 0;

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

        void createFramebuffer();

        void createDiffuseIrradianceMap();
        void createPrefilteredEnvironmentMap();
        void createIntegratedBRDFMap();

        void loadDiffuseIrradianceProgram();
        void loadPrefilteredEnvironmentProgram();
        void loadIntegrateBRDFProgram();

        // Input: Environment cubemap texture
        // Output: Sets the diffuseIrradianceTexture to the convolved environmentMap
        void renderToDiffuseIrradiance();
        void renderToPrefilterMap();
        void renderToIntegratedBRDFMap();
};
