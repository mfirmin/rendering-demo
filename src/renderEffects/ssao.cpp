#include "ssao.hpp"

#include "gl/shaderUtils.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>

std::uniform_real_distribution<float> randomFloat(0.0f, 1.0f);
std::default_random_engine generator;

SSAOEffect::SSAOEffect(int w, int h) :
    width(w),
    height(h),
    blurEffect(w, h)
{}

SSAOEffect::~SSAOEffect() {
    glDeleteTextures(1, &ambientOcclusionTexture);
    glDeleteTextures(1, &kernelNoiseTexture);
    glDeleteFramebuffers(1, &fbo);

    glDeleteProgram(program);
    glDeleteProgram(debugProgram);
}

// Must call this AFTER GL/SDL have been initialized
void SSAOEffect::initialize() {
    createGLObjects();
    constructKernel();
    constructKernelNoise();
    createProgram();
    createDebugProgram();

    blurEffect.initialize();
}

void SSAOEffect::createGLObjects() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &ambientOcclusionTexture);
    glBindTexture(GL_TEXTURE_2D, ambientOcclusionTexture);
    // just need red component (greyscale)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ambientOcclusionTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error creating SSAO: Error creating framebuffer\n";
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOEffect::constructKernel() {
    auto lerp = [](auto a, auto b, auto f) { return a + f * (b - a); };

    for (unsigned int i = 0; i < 64; i++) {
        glm::vec3 sample(
            randomFloat(generator) * 2.0f - 1.0f,
            randomFloat(generator) * 2.0f - 1.0f,
            randomFloat(generator)
        );

        float scale = static_cast<float>(i) / 64.0f;

        scale = lerp(0.1f, 1.0f, scale * scale);

        sample = glm::normalize(sample) * randomFloat(generator) * scale;
        kernel.push_back(sample);
    }
}

void SSAOEffect::constructKernelNoise() {
    std::vector<glm::vec3> noise;

    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 value(
            randomFloat(generator) * 2.0f - 1.0f,
            randomFloat(generator) * 2.0f - 1.0f,
            0.0f
        );

        noise.push_back(value);
    }

    glGenTextures(1, &kernelNoiseTexture);
    glBindTexture(GL_TEXTURE_2D, kernelNoiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void SSAOEffect::createProgram() {
    std::string vertexShader = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    std::string fragmentShader = R"(
        #version 330

        const int numSamples = 64;

        // scene is an hdr, floating point texture
        uniform sampler2D gPosition;
        uniform sampler2D gNormal;
        uniform sampler2D noise;

        uniform vec3 samples[numSamples];

        uniform mat4 projectionMatrix;

        uniform float width;
        uniform float height;

        // Adjust to use only a subset of the samples.
        // default = numSamples
        uniform int samplesToUse;
        // multiplier on kernel size. 1.0f by default.
        uniform float radius;
        // bias to use when testing for occlusion
        uniform float bias;

        in vec2 vUv;

        out float fragColor;

        void main() {
            vec2 noiseScale = vec2(width / 4.0, height / 4.0);

            vec3 position = texture(gPosition, vUv).xyz;
            vec3 normal = texture(gNormal, vUv).xyz;
            vec3 randomVec = texture(noise, vUv * noiseScale).xyz;

            vec3 v1 = normalize(randomVec - normal * dot(randomVec, normal));
            vec3 v2 = cross(normal, v1);
            mat3 sampleBasis =  mat3(v1, v2, normal);

            float occlusion = 0.0;
            for (int i = 0; i < samplesToUse; i++) {
                // convert the sample from tangent space to
                // world (view in this case) space
                vec3 sample = sampleBasis * samples[i];

                sample = position + sample * radius;

                // convert to clip space (then NDC)
                // so we can look up other fragment positions
                vec4 offset = projectionMatrix * vec4(sample, 1.0);
                offset.xyz /= offset.w;
                // convert from NDC to 0->1 (texture space)
                offset.xyz = (offset.xyz + vec3(1.0)) * 0.5;

                // get the (stored) depth of the fragment corresponding
                // to the sample position. Since this is in eyespace,
                // depth = z component
                float sampleDepth = texture(gPosition, offset.xy).z;
                // put less weight on samples with depths significantly outside of the sample radius
                float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
                occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
            }

            // closer to 1.0 = more occluded
            float occlusionPercent = occlusion / samplesToUse;
            // subtract from 1 so that black = occluded, white = not
            fragColor = 1.0 - occlusionPercent;
        }
    )";

    program = ShaderUtils::compile(vertexShader, fragmentShader);

    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "width"), width);
    glUniform1f(glGetUniformLocation(program, "height"), height);
    glUniform1f(glGetUniformLocation(program, "radius"), 0.5f);
    glUniform1f(glGetUniformLocation(program, "bias"), 0.025f);

    glUniform1i(glGetUniformLocation(program, "samplesToUse"), kernel.size());

    std::vector<float> flatKernel(kernel.size() * 3);

    for (unsigned int i = 0; i < kernel.size(); i++) {
        flatKernel[i * 3 + 0] = kernel[i].x;
        flatKernel[i * 3 + 1] = kernel[i].y;
        flatKernel[i * 3 + 2] = kernel[i].z;
    }

    glUniform3fv(glGetUniformLocation(program, "samples"), kernel.size(), flatKernel.data());
    glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUseProgram(0);
}

void SSAOEffect::createDebugProgram() {
    std::string vertexShader = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    std::string fragmentShader = R"(
        #version 330

        uniform sampler2D ambientOcclusion;

        in vec2 vUv;

        out vec4 fragColor;

        void main() {
            vec4 ao = texture(ambientOcclusion, vUv);
            fragColor = vec4(ao.rrr, 1.0);
        }
    )";

    debugProgram = ShaderUtils::compile(vertexShader, fragmentShader);
}

void SSAOEffect::setProjectionMatrix(const glm::mat4& projectionMatrix) const {
    glUseProgram(program);
    auto viewMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUseProgram(0);
}

void SSAOEffect::render(GLuint vao, GLuint gPosition, GLuint gNormal) const {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // no depth buffer, so no need to clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    // bind the position texture to texture slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glUniform1i(glGetUniformLocation(program, "gPosition"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glUniform1i(glGetUniformLocation(program, "gNormal"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, kernelNoiseTexture);
    glUniform1i(glGetUniformLocation(program, "noise"), 2);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);

    // blur the ambientOcclusionTexture
    blurEffect.render(vao, ambientOcclusionTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Now ambientOcclusionTexture contains the ambient occlusion,
    // and blurEffect's result contains the blurred texture
    // and is ready for the lighting pass
}

// render the ambient occlusion texture to the screen
void SSAOEffect::renderDebug(GLuint vao) const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(debugProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurEffect.getResult());
    glUniform1i(glGetUniformLocation(debugProgram, "ambientOcclusion"), 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}
