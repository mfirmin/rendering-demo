#include "ibl.hpp"

#include "gl/shaderUtils.hpp"

#include <fstream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>


IBL::IBL() {}

void IBL::initialize(GLuint em) {
    createTexture();
    createProgram();

    cubeMesh.fromOBJ("assets/unit_cube.obj");

    setEnvironmentMap(em);
}


IBL::~IBL() {
    // TODO: Free buffers
}

void IBL::createTexture() {
    // Generate the framebuffer for rendering/creating the cubemap
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Need a depth buffer too
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    // Generate the diffuseIrradiance texture to render into
    glGenTextures(1, &diffuseIrradianceTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, diffuseIrradianceTexture);

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

void IBL::createProgram() {
    std::string vShader;
    std::string fShader;

    std::ifstream ifs;
    ifs.open("assets/shaders/diffuseIrradiance.vert");

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

    ifs.open("assets/shaders/diffuseIrradiance.frag");

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

    diffuseIrradianceProgram = ShaderUtils::compile(vShader, fShader);

    if (diffuseIrradianceProgram == 0) {
        std::cout << "Failed to compile program\n";
        return;
    }
}

// render to each of the six faces of the diffuseIrradiance
void IBL::renderToDiffuseIrradiance() {
    glCullFace(GL_FRONT);
    glUseProgram(diffuseIrradianceProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

    glUniform1i(glGetUniformLocation(diffuseIrradianceProgram, "environmentMap"), 0);

    glUniformMatrix4fv(glGetUniformLocation(diffuseIrradianceProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (unsigned int i = 0; i < CUBE_FACES; i++) {
        glUniformMatrix4fv(glGetUniformLocation(diffuseIrradianceProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrices.at(i)));

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, diffuseIrradianceTexture, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(cubeMesh.getVertexArrayObject());
        glDrawArrays(GL_TRIANGLES, 0, cubeMesh.getVertexCount());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glCullFace(GL_BACK);
}

void IBL::setEnvironmentMap(GLuint em) {
    environmentMap = em;

    renderToDiffuseIrradiance();
}
