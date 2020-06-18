#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <memory>
#include <vector>

class Light;

class DeferredPBREffect {
    public:
        DeferredPBREffect(int width, int height);

        DeferredPBREffect(DeferredPBREffect&& other) = default;
        DeferredPBREffect& operator=(DeferredPBREffect&& other) = default;

        DeferredPBREffect(const DeferredPBREffect& other) = delete;
        DeferredPBREffect& operator=(const DeferredPBREffect& other) = delete;

        ~DeferredPBREffect();

        void initialize();

        GLuint getDebugProgram() {
            return debugProgram;
        }

        GLuint getProgram() {
            return program;
        }

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

        GLuint getEmissive() {
            return emissiveTexture;
        }

        GLuint getOutputFramebuffer() {
            return outputFbo;
        }

        GLuint getOutputTexture() {
            return outputTexture;
        }

        void setLights(const std::vector<std::shared_ptr<Light>>& lights);

        void setViewMatrix(
            const glm::mat4& viewMatrix
        );

        void toggleSSAO(bool value);
        void toggleIBL(bool value);

        void render(
            GLuint vao,
            GLuint ambientOcclusion,
            GLuint diffuseIrradianceMap,
            GLuint prefilteredEnvironmentMap,
            GLuint integratedBRDFMap
        );
    private:
        int width;
        int height;

        GLuint fbo = 0;
        GLuint positionTexture = 0;
        GLuint normalTexture = 0;
        GLuint albedoTexture = 0;
        GLuint emissiveTexture = 0;
        GLuint roughnessAndMetalnessTexture = 0;

        GLuint depthBuffer = 0;

        GLuint outputFbo = 0;
        GLuint outputTexture = 0;

        GLuint program = 0;
        GLuint debugProgram = 0;

        void createDebugProgram();
        void createProgram();

        void createOutput();
};
