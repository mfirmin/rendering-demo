#include "hdri.hpp"

#include "gl/shaderUtils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fstream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>


HDRI::HDRI() {}

void HDRI::initialize(std::string f) {
    filename = f;
    loadTexture();
    createCubemap();
    createProgram();

    cubeMesh.fromOBJ("assets/unit_cube.obj");

    renderToCubemap();
}


HDRI::~HDRI() {
    glDeleteTextures(1, &texture);
    glDeleteTextures(1, &cubemapTexture);
    glDeleteRenderbuffers(1, &depthBuffer);

    glDeleteFramebuffers(1, &fbo);

    glDeleteProgram(cubemapProgram);
}

void HDRI::loadTexture() {
    stbi_set_flip_vertically_on_load(true);

    float* data = stbi_loadf(filename.c_str(), &width, &height, &numChannels, 0);

    if (data == nullptr || numChannels != 3) {
        std::cout << "Error loading HDR image\n";
        return;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void HDRI::createCubemap() {
    // Generate the framebuffer for rendering/creating the cubemap
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Need a depth buffer too
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);


    // Generate the cubemap texture to render into
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    for (unsigned int i = 0; i < CUBE_FACES; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr
        );
    }


    // ensure we don't repeat
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void HDRI::createProgram() {
    std::string vShader;
    std::string fShader;

    std::ifstream ifs;
    ifs.open("assets/shaders/equirectangularToCube.vert");

    if (ifs.fail()) {
        std::cout << "Could not open vShader file\n";
        return;
    }

    ifs.seekg(0, std::ios::end);
    vShader.reserve(ifs.tellg());
    ifs.seekg(0, std::ios::beg);

    vShader.assign(
        std::istreambuf_iterator<char>(ifs),
        std::istreambuf_iterator<char>()
    );

    ifs.close();

    ifs.open("assets/shaders/equirectangularToCube.frag");

    if (ifs.fail()) {
        std::cout << "Could not open fShader file\n";
        return;
    }

    ifs.seekg(0, std::ios::end);
    fShader.reserve(ifs.tellg());
    ifs.seekg(0, std::ios::beg);

    fShader.assign(
        std::istreambuf_iterator<char>(ifs),
        std::istreambuf_iterator<char>()
    );

    ifs.close();

    cubemapProgram = ShaderUtils::compile(vShader, fShader);

    if (cubemapProgram == 0) {
        std::cout << "Failed to compile program\n";
        return;
    }
}

// render to each of the six faces of the cubemap
void HDRI::renderToCubemap() {
    glCullFace(GL_FRONT);
    glUseProgram(cubemapProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniform1i(glGetUniformLocation(cubemapProgram, "equirectangularMap"), 0);

    glUniformMatrix4fv(glGetUniformLocation(cubemapProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (unsigned int i = 0; i < CUBE_FACES; i++) {
        glUniformMatrix4fv(glGetUniformLocation(cubemapProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrices.at(i)));

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapTexture, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(cubeMesh.getVertexArrayObject());
        glDrawArrays(GL_TRIANGLES, 0, cubeMesh.getVertexCount());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glCullFace(GL_BACK);
}

void HDRI::setProjectionAndViewMatrices(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
    glUseProgram(cubemapProgram);
    auto projectionMatrixLocation = glGetUniformLocation(cubemapProgram, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(cubemapProgram, "viewMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUseProgram(0);
}

void HDRI::renderCube() {
    glUseProgram(cubemapProgram);
    glCullFace(GL_FRONT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniform1i(glGetUniformLocation(cubemapProgram, "equirectangularMap"), 0);

    glBindVertexArray(cubeMesh.getVertexArrayObject());
    glDrawArrays(GL_TRIANGLES, 0, cubeMesh.getVertexCount());

    glCullFace(GL_BACK);
    glUseProgram(0);
}
