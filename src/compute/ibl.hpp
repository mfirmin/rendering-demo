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

        GLuint getDiffuseIrradiance() const {
            return diffuseIrradianceMap;
        }

        GLuint getPrefilteredMap() const {
            return prefilterMap;
        }

        GLuint getIntegratedBRDFMap() const {
            return integratedBRDFMap;
        }

        void setEnvironmentMap(GLuint em);

        ~IBL();
    private:
        static constexpr unsigned int CUBE_FACES = 6;
        // Since diffuse irradiance doesn't vary much over N, we can store
        // a very low detailed texture
        static const unsigned int DIFFUSE_IRRADIANCE_TEXTURE_WIDTH;
        static const unsigned int DIFFUSE_IRRADIANCE_TEXTURE_HEIGHT;

        static const unsigned int PREFILTERED_TEXTURE_MIPMAP_LEVELS;
        static const unsigned int PREFILTERED_TEXTURE_WIDTH;
        static const unsigned int PREFILTERED_TEXTURE_HEIGHT;

        static const unsigned int INTEGRATED_BRDF_TEXTURE_WIDTH;
        static const unsigned int INTEGRATED_BRDF_TEXTURE_HEIGHT;

        // lookAt(position, target, up)
        static const std::array<glm::mat4, CUBE_FACES> VIEW_MATRICES;

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
