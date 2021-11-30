#include "ibl.hpp"

#include "gl/shaderUtils.hpp"

#include <fstream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>


IBL::IBL() {}

void IBL::initialize(GLuint em, GLuint vao) {
    screenVertexArray = vao;

    createFramebuffer();

    createDiffuseIrradianceMap();
    createPrefilteredEnvironmentMap();
    createIntegratedBRDFMap();

    loadDiffuseIrradianceProgram();
    loadPrefilteredEnvironmentProgram();
    loadIntegrateBRDFProgram();

    cubeMesh.fromOBJ("assets/unit_cube.obj");

    setEnvironmentMap(em);
}


IBL::~IBL() {
    glDeleteTextures(1, &diffuseIrradianceMap);
    glDeleteTextures(1, &prefilterMap);
    glDeleteTextures(1, &integratedBRDFMap);
    glDeleteRenderbuffers(1, &depthBuffer);

    glDeleteFramebuffers(1, &fbo);

    glDeleteProgram(diffuseIrradianceProgram);
    glDeleteProgram(prefilterProgram);
    glDeleteProgram(integrateBRDFProgram);
}

void IBL::createFramebuffer() {
    // Generate the framebuffer for rendering/creating the cubemap
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Need a depth buffer too
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

    // note that we will change the size when rendering the other buffer
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, DIFFUSE_IRRADIANCE_TEXTURE_WIDTH, DIFFUSE_IRRADIANCE_TEXTURE_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
}

void IBL::createDiffuseIrradianceMap() {
    // Generate the diffuseIrradiance texture to render into
    glGenTextures(1, &diffuseIrradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, diffuseIrradianceMap);

    for (unsigned int i = 0; i < CUBE_FACES; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            DIFFUSE_IRRADIANCE_TEXTURE_WIDTH, DIFFUSE_IRRADIANCE_TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr
        );
    }

    // ensure we don't repeat
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void IBL::createPrefilteredEnvironmentMap() {
    // Generate the prefilter texture to render into
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

    for (unsigned int i = 0; i < CUBE_FACES; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            PREFILTERED_TEXTURE_WIDTH, PREFILTERED_TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr
        );
    }

    // ensure we don't repeat
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void IBL::createIntegratedBRDFMap() {
    // Generate the diffuseIrradiance texture to render into
    glGenTextures(1, &integratedBRDFMap);
    glBindTexture(GL_TEXTURE_2D, integratedBRDFMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, INTEGRATED_BRDF_TEXTURE_WIDTH, INTEGRATED_BRDF_TEXTURE_HEIGHT, 0, GL_RG, GL_FLOAT, 0);

    // ensure we don't repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void IBL::loadDiffuseIrradianceProgram() {
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

void IBL::loadPrefilteredEnvironmentProgram() {
    std::string vShader;
    std::string fShader;

    std::ifstream ifs;
    ifs.open("assets/shaders/prefilter.vert");

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

    ifs.open("assets/shaders/prefilter.frag");

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

    prefilterProgram = ShaderUtils::compile(vShader, fShader);

    if (prefilterProgram == 0) {
        std::cout << "Failed to compile program\n";
        return;
    }
}

void IBL::loadIntegrateBRDFProgram() {
    std::string vShader;
    std::string fShader;

    std::ifstream ifs;
    ifs.open("assets/shaders/integratedBRDF.vert");

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

    ifs.open("assets/shaders/integratedBRDF.frag");

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

    integrateBRDFProgram = ShaderUtils::compile(vShader, fShader);

    if (integrateBRDFProgram == 0) {
        std::cout << "Failed to compile program\n";
        return;
    }
}

// render to each of the six faces of the diffuseIrradiance
void IBL::renderToDiffuseIrradiance() {
    // ensure we set the depthbuffer to the proper size
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, DIFFUSE_IRRADIANCE_TEXTURE_WIDTH, DIFFUSE_IRRADIANCE_TEXTURE_HEIGHT);

    glViewport(0, 0, DIFFUSE_IRRADIANCE_TEXTURE_WIDTH, DIFFUSE_IRRADIANCE_TEXTURE_HEIGHT);

    glCullFace(GL_FRONT);
    glUseProgram(diffuseIrradianceProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

    glUniform1i(glGetUniformLocation(diffuseIrradianceProgram, "environmentMap"), 0);

    glUniformMatrix4fv(glGetUniformLocation(diffuseIrradianceProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (unsigned int i = 0; i < CUBE_FACES; i++) {
        glUniformMatrix4fv(glGetUniformLocation(diffuseIrradianceProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrices.at(i)));

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, diffuseIrradianceMap, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(cubeMesh.getVertexArrayObject());
        glDrawArrays(GL_TRIANGLES, 0, cubeMesh.getVertexCount());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glCullFace(GL_BACK);
}

void IBL::renderToPrefilterMap() {
    // ensure we set the depthbuffer to the proper size
    glCullFace(GL_FRONT);
    glUseProgram(prefilterProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

    glUniform1i(glGetUniformLocation(prefilterProgram, "environmentMap"), 0);

    glUniformMatrix4fv(glGetUniformLocation(prefilterProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (unsigned int mipmapLevel = 0; mipmapLevel < PREFILTERED_TEXTURE_MIPMAP_LEVELS; mipmapLevel++) {
        const float oneHalf = 0.5f;

        unsigned int mipmapWidth = static_cast<unsigned int>(PREFILTERED_TEXTURE_WIDTH * std::pow(oneHalf, mipmapLevel));
        unsigned int mipmapHeight = static_cast<unsigned int>(PREFILTERED_TEXTURE_HEIGHT * std::pow(oneHalf, mipmapLevel));

        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipmapWidth, mipmapHeight);

        glViewport(0, 0, mipmapWidth, mipmapHeight);

        float roughness = static_cast<float>(mipmapLevel) / (static_cast<float>(PREFILTERED_TEXTURE_MIPMAP_LEVELS) - 1.0f);

        glUniform1f(glGetUniformLocation(prefilterProgram, "roughness"), roughness);

        for (unsigned int i = 0; i < CUBE_FACES; i++) {
            glUniformMatrix4fv(glGetUniformLocation(prefilterProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrices.at(i)));

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mipmapLevel);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(cubeMesh.getVertexArrayObject());
            glDrawArrays(GL_TRIANGLES, 0, cubeMesh.getVertexCount());
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glCullFace(GL_BACK);
}

void IBL::renderToIntegratedBRDFMap() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, INTEGRATED_BRDF_TEXTURE_WIDTH, INTEGRATED_BRDF_TEXTURE_HEIGHT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, integratedBRDFMap, 0);

    glViewport(0, 0, INTEGRATED_BRDF_TEXTURE_WIDTH, INTEGRATED_BRDF_TEXTURE_HEIGHT);
    glUseProgram(integrateBRDFProgram);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(screenVertexArray);

    // render quad here
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void IBL::setEnvironmentMap(GLuint em) {
    environmentMap = em;

    renderToDiffuseIrradiance();
    renderToPrefilterMap();
    renderToIntegratedBRDFMap();
}
