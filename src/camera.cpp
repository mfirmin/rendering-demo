#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(
    float aspect,
    float fov,
    glm::vec3 position,
    glm::vec3 target,
    glm::vec3 up,
    float near,
    float far
) :
    aspect(aspect),
    fov(fov),
    position(position),
    target(target),
    up(up),
    near(near),
    far(far)
{}

Camera::~Camera() {}

glm::mat4 Camera::getProjectionMatrix() {
    return glm::perspective(fov, aspect, near, far);
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, target, up);
}
