#version 330 core
layout (location = 0) in vec3 position;

out vec3 vPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main() {
    vPosition = position;
    gl_Position = projectionMatrix * viewMatrix * vec4(vPosition, 1.0);
}
