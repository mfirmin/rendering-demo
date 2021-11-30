#include "skyboxDeferred.hpp"

#include "gl/shaderUtils.hpp"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <string>
#include <sstream>

SkyboxDeferredMaterial::SkyboxDeferredMaterial(GLuint cubemap) :
    Material(glm::vec3(0.0f, 0.0f, 0.0f), 0, 0),
    cubemap(cubemap)
{}

void SkyboxDeferredMaterial::create() {
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
        out vec4 vPositionEyespace;

        void main() {
            vPosition = position;

            // Remove any translation from the view matrix
            mat4 rotView = mat4(mat3(viewMatrix));

            vPositionEyespace = rotView * vec4(vPosition, 1.0);
            vec4 clipPosition = projectionMatrix * vPositionEyespace;

            // xyww ensures depth = 1.0 (furthest possible)
            gl_Position = clipPosition.xyww;
        }
    )";

    std::string fragmentShaderSource = R"(
        #version 330

        layout(location = 0) out vec4 position;
        layout(location = 2) out vec4 albedo;

        in vec3 vPosition;
        in vec3 vPositionEyespace;

        uniform samplerCube cubemap;

        void main() {
            vec3 color = texture(cubemap, vPosition).rgb;

            // don't do tone mapping/gamma mapping here, as they are done in a later render pass

            position = vec4(position.xyz, 0.0);
            albedo = vec4(color, 1.0);
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

void SkyboxDeferredMaterial::setUniforms() const {
    GLuint shader = getProgram();
    glUseProgram(shader);
    // bind the cubemap to texture slot 0 and update the uniform accordingly
    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glUniform1i(glGetUniformLocation(shader, "cubemap"), 0);
    glUseProgram(0);
}

