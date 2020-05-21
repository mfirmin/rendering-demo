#pragma once

#include <glm/glm.hpp>

constexpr float DEFAULT_NEAR = 0.01f;
constexpr float DEFAULT_FAR = 100.0f;

// TODO: Orthographic Camera support
class Camera {
    public:
        Camera(
            float aspect,
            float fov,
            glm::vec3 position,
            glm::vec3 target,
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float near = DEFAULT_NEAR,
            float far = DEFAULT_FAR
        );

        Camera(Camera&& other) = default;
        Camera(const Camera& other) = default;

        Camera& operator=(const Camera& other) = default;
        Camera& operator=(Camera&& other) = default;

        ~Camera();

        glm::mat4 getProjectionMatrix();
        glm::mat4 getViewMatrix();
    private:
        float aspect;
        float fov;

        glm::vec3 position;
        glm::vec3 target;
        glm::vec3 up;

        float near;
        float far;
};
