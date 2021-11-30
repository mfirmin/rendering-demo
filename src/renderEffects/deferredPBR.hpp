#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <memory>
#include <vector>

class Light;

// TODO(mfirmin): This and DeferredShadingEffect should both inherit from a shared parent class
class DeferredPBREffect {
    public:
        DeferredPBREffect(int width, int height);

        DeferredPBREffect(DeferredPBREffect&& other) = default;
        DeferredPBREffect& operator=(DeferredPBREffect&& other) = default;

        DeferredPBREffect(const DeferredPBREffect& other) = delete;
        DeferredPBREffect& operator=(const DeferredPBREffect& other) = delete;

        ~DeferredPBREffect();

        void initialize();

        GLuint getDebugProgram() const {
            return debugProgram;
        }

        GLuint getProgram() const {
            return program;
        }

        GLuint getFramebuffer() const {
            return fbo;
        }

        GLuint getPosition() const {
            return positionTexture;
        }

        GLuint getNormal() const {
            return normalTexture;
        }

        GLuint getAlbedo() const {
            return albedoTexture;
        }

        GLuint getEmissive() const {
            return emissiveTexture;
        }

        GLuint getOutputFramebuffer() const {
            return outputFbo;
        }

        GLuint getOutputTexture() const {
            return outputTexture;
        }

        void setLights(const std::vector<std::shared_ptr<Light>>& lights) const;

        void setViewMatrix(
            const glm::mat4& viewMatrix
        ) const;

        void toggleSSAO(bool value) const;
        void toggleIBL(bool value) const;

        void render(
            GLuint vao,
            GLuint ambientOcclusion,
            GLuint diffuseIrradianceMap,
            GLuint prefilteredEnvironmentMap,
            GLuint integratedBRDFMap
        ) const;
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
