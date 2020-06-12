#include "skybox.hpp"

#include "gl/shaderUtils.hpp"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <string>
#include <sstream>

SkyboxMaterial::SkyboxMaterial(GLuint cubemap) :
    Material(glm::vec3(0.0f, 0.0f, 0.0f), 0, 0),
    cubemap(cubemap)
{}

void SkyboxMaterial::create() {
    if (getProgram() != 0) {
        // already initialized
        return;
    }
    std::string vertexShaderSource = R"(
        #version 330
        layout(location = 0) in vec3 position;

        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;

        out vec3 vPosition;

        void main() {
            vPosition = position;

            // Remove any translation from the view matrix
            mat4 rotView = mat4(mat3(viewMatrix));
            vec4 clipPosition = projectionMatrix * rotView * vec4(vPosition, 1.0);

            // xyww ensures depth = 1.0 (furthest possible)
            gl_Position = clipPosition.xyww;
        }
    )";

    std::string fragmentShaderSource = R"(
        #version 330

        out vec4 fragColor;

        in vec3 vPosition;

        uniform samplerCube cubemap;

        void main() {
            vec3 color = texture(cubemap, vPosition).rgb;

            // don't do tone mapping/gamma mapping here, as they are done in a later render pass

            fragColor = vec4(color, 1.0);
        }
    )";

    if (!compile(vertexShaderSource, fragmentShaderSource)) {
        return;
    }

    GLuint shader = getProgram();

    glUseProgram(shader);
    auto projectionMatrixLocation = glGetUniformLocation(shader, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(shader, "viewMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniform1i(glGetUniformLocation(shader, "cubemap"), 0);
    glUseProgram(0);
}

void SkyboxMaterial::setUniforms() {
    GLuint shader = getProgram();
    glUseProgram(shader);
    // bind the cubemap to texture slot 0 and update the uniform accordingly
    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glUniform1i(glGetUniformLocation(shader, "cubemap"), 0);
    glUseProgram(0);
}

SkyboxMaterial::~SkyboxMaterial() {}

