#include "deferredPBR.hpp"

#include "gl/shaderUtils.hpp"
#include "light/light.hpp"

#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <sstream>

DeferredPBREffect::DeferredPBREffect(int w, int h) :
    width(w), height(h)
{
}

void DeferredPBREffect::initialize() {
    // initialize the framebuffer for the render target
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    /** Position Texture **/

    // initialize the texture to bind to the buffer
    glGenTextures(1, &positionTexture);
    glBindTexture(GL_TEXTURE_2D, positionTexture);

    // floating point texture, RGB for position, A for depth
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTexture, 0);

    /** Normal Texture **/

    glGenTextures(1, &normalTexture);
    glBindTexture(GL_TEXTURE_2D, normalTexture);

    // floating point texture, RGB for position, A for depth
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);

    /** Albedo Texture **/

    glGenTextures(1, &albedoTexture);
    glBindTexture(GL_TEXTURE_2D, albedoTexture);

    // floating point texture, RGB for position, A for depth
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedoTexture, 0);

    /** Emissive Texture **/

    glGenTextures(1, &emissiveTexture);
    glBindTexture(GL_TEXTURE_2D, emissiveTexture);

    // floating point texture, RGB for position, A for depth
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, emissiveTexture, 0);

    /** Roughness And Metalness Texture **/

    glGenTextures(1, &roughnessAndMetalnessTexture);
    glBindTexture(GL_TEXTURE_2D, roughnessAndMetalnessTexture);

    // floating point texture, R for roughness, G for metalness
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, roughnessAndMetalnessTexture, 0);


    // initialize the depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // attach the depth buffer to the frame buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    std::array<GLenum, 5> drawbuffers = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };


    glDrawBuffers(5, drawbuffers.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error creating DeferredPBREffect: Error creating framebuffer\n";
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    createOutput();
    createDebugProgram();
    createProgram();
}

DeferredPBREffect::~DeferredPBREffect() {
    // TODO: Free buffers
}

void DeferredPBREffect::createOutput() {
    glGenFramebuffers(1, &outputFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);

    // initialize the texture to bind to the buffer
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);

    // floating point texture, RGB for position, A for depth
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

    std::array<GLenum, 1> drawbuffers = { GL_COLOR_ATTACHMENT0 };


    glDrawBuffers(1, drawbuffers.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error creating DeferredPBREffect: Error creating output framebuffer\n";
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredPBREffect::createDebugProgram() {
    std::string vertexShaderSource = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";


    std::string fragmentShaderSource = R"(
        #version 330

        // scene is a floating point (HDR) texture
        uniform sampler2D input;

        in vec2 vUv;

        out vec4 fragColor;

        void main() {
            vec3 color = texture(input, vUv).rgb;

            fragColor = vec4(color, 1.0);
        }
    )";

    debugProgram = ShaderUtils::compile(vertexShaderSource, fragmentShaderSource);
}

void DeferredPBREffect::createProgram() {
    std::string vertexShaderSource = R"(
        #version 330
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;

        out vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";


    std::string fragmentShaderSource = R"(
        #version 330

        #define MAX_LIGHTS 10
        #define PI 3.1415926535

        uniform mat4 viewMatrix;

        uniform float emissiveEnabled;
        uniform float ssaoEnabled;
        uniform float iblEnabled;

        uniform int numLights;
        uniform struct Light {
            vec4 position;
            vec3 color;
            float intensity;
            float ambientCoefficient;
            float attenuation;
            float enabled;
            // spotlight only
            float coneAngle;
            vec3 coneDirection;
        } lights[MAX_LIGHTS];

        // scene is a floating point (HDR) texture
        uniform sampler2D gPosition;
        uniform sampler2D gNormal;
        uniform sampler2D gAlbedo;
        uniform sampler2D gEmissive;
        uniform sampler2D gRoughnessAndMetalness;
        uniform sampler2D ambientOcclusion;

        // IBL
        uniform samplerCube diffuseIrradianceMap;
        // Specular IBL
        uniform samplerCube prefilteredEnvironmentMap;
        uniform sampler2D integratedBRDFMap;

        in vec2 vUv;

        out vec4 fragColor;

        // Trwobridge-Reitz GGX Normal Distribution Function
        //
        // Determines the ratio of microfacets that are aligned
        // to the Halfway vector using the provided roughness value
        float NDF(vec3 N, vec3 H, float r) {
            float nDotH = max(dot(N, H), 0.0);
            float r2 = r * r;
            float denom = (nDotH * nDotH * (r2 - 1.0) + 1.0);
            denom = denom * denom * PI;

            return r2 / denom;
        }

        // Schlick-GGX Approximation to the geometry function
        // Determines the ratio of microfacets that are overshadowed
        // by others
        float geometrySchlick(vec3 N, vec3 V, float k) {
            float nDotV = max(dot(N, V), 0.0);

            return nDotV / (nDotV * (1.0 - k) + k);
        }

        // Smith method uses the Schlick method to compute
        // overshadowing both from the light ray and to the
        // viewpoint
        float geometrySmith(vec3 N, vec3 V, vec3 L, float r) {
            // For direct lighting:
            float k = (r + 1.0) * (r + 1.0) / 8.0;
            // For IBL:
            // float k = r * r / 2.0;

            return geometrySchlick(N, V, k) * geometrySchlick(N, L, k);
        }

        // note: this isn't the cosTheta I thought it was.
        // it's actually dot(H, V)
        vec3 fresnelSchlick(float cosTheta, vec3 F0) {
            // fix if cosTheta > 1.0
            cosTheta = min(cosTheta, 1.0);
            return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
        }

        // variation of fresnelSchlick accounting for roughness, used by IBL
        // as we don't have a halfway vector for IBL
        vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
            cosTheta = min(cosTheta, 1.0);
            return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
        }

        vec3 fLambert(vec3 albedo) {
            return albedo / PI;
        }

        // wo = direction to viewpoint
        // wi = direction to lightsource
        vec3 fCookTorrance(vec3 V, vec3 L, vec3 N, vec3 albedo, vec3 F0, float roughness, float metalness) {
            vec3 H = normalize(L + V);

            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
            float G = geometrySmith(N, V, L, roughness);
            float D = NDF(N, H, roughness);

            vec3 ks = F;
            vec3 kd = vec3(1.0) - ks;
            kd *= (1.0 - metalness);

            vec3 lambertDiffuse = kd * fLambert(albedo);

            float denom = (4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0));
            // ensure no divide by 0
            denom = max(denom, 0.001);
            // note that ks is already included in this (it is F)
            vec3 specularCT = (D * F * G) / denom;

            return lambertDiffuse + specularCT;
        }

        vec3 illuminate(vec4 albedo, vec4 emissive, vec3 P, vec3 N, vec3 V, float roughness, float metalness) {
            vec3 inColor = albedo.rgb;
            vec3 emissiveColor = emissive.rgb;
            float emissiveStrength = emissive.a;

            // use F0 = 0.04 for dielectric surfaces (non-metallic)
            vec3 F0 = vec3(0.04);
            // for metallic surfaces, mix it towards the albedo of the surface
            F0 = mix(F0, inColor, metalness);

            vec3 outColor = vec3(0.0);

            for (int i = 0; i < numLights; i++) {
                Light light = lights[i];
                if (light.enabled < 0.5) {
                    continue;
                }
                vec3 L;
                float attenuation;
                if (light.position.w == 0.0) {
                    // directional light
                    // convert the direction into eyespace
                    L = normalize((viewMatrix * light.position).xyz);
                    // directional lights don't support attenuation
                    attenuation = 1.0;
                } else {
                    vec3 lightPositionEyespace = (viewMatrix * light.position).xyz;
                    L = normalize(lightPositionEyespace - P);
                    float distance = length(lightPositionEyespace - P);
                    attenuation = 1.0 / (1.0 + light.attenuation * pow(distance, 2));
                }

                vec3 radiance = light.color * light.intensity * attenuation;

                float nDotL = max(dot(N, L), 0.0);

                outColor += fCookTorrance(V, L, N, inColor, F0, roughness, metalness) * radiance * nDotL;
            }

            float ao = 1.0;

            if (ssaoEnabled > 0.5f) {
                ao = texture(ambientOcclusion, vUv).r;
            }

            if (iblEnabled < 0.5f) {
                // improvised ambient term, independent of light sources
                vec3 ambient = vec3(0.03) * inColor * ao;

                outColor += ambient;
            } else {
                float nDotV = max(dot(N, V), 0.0);
                vec3 kS = fresnelSchlickRoughness(nDotV, F0, roughness);
                vec3 kD = 1.0 - kS;
                kD *= (1.0 - metalness);

                vec3 irradiance = texture(diffuseIrradianceMap, N).rgb;
                // diffuse term is scene irradiance * albedo scaled by ambient occlusion
                // Note that diffuse and ambient are now combined into one term,
                // rather than having a separate ambient term (which was a hack anyway)
                vec3 diffuse = kD * irradiance * inColor;

                vec3 R = reflect(-V, N);
                const float MAX_REFLECTION_LOD = 4.0;
                vec3 prefilteredColor = textureLod(prefilteredEnvironmentMap, R, roughness * MAX_REFLECTION_LOD).rgb;
                vec2 envBRDF = texture(integratedBRDFMap, vec2(nDotV, roughness)).rg;
                vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

                outColor += (diffuse + specular) * ao;
            }

            if (emissiveEnabled > 0.5f) {
                outColor += emissiveStrength * emissiveColor;
            }

            return outColor;
        }

        void main() {
            vec4 p = texture(gPosition, vUv);
            vec3 position = p.xyz;
            vec3 normal = texture(gNormal, vUv).rgb;
            vec4 albedo = texture(gAlbedo, vUv);
            vec4 emissive = texture(gEmissive, vUv);
            vec2 mr = texture(gRoughnessAndMetalness, vUv).rg;

            vec3 N = normalize(normal);
            vec3 V = normalize(-position);

            if (dot(N, V) < 0.0) {
                N = -N;
            }

            vec3 color = albedo.rgb;
            // only apply illumination if position's w component is 1
            // For instance, the skybox should not be illuminated
            if (p.w > 0.5f) {
                color = illuminate(albedo, emissive, position, N, V, max(mr.r, 0.01), mr.g);
            }

            fragColor = vec4(color, 1.0);
        }
    )";

    program = ShaderUtils::compile(vertexShaderSource, fragmentShaderSource);

    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "ssaoEnabled"), 1.0f);
    glUniform1f(glGetUniformLocation(program, "emissiveEnabled"), 1.0f);
    glUniform1f(glGetUniformLocation(program, "iblEnabled"), 1.0f);
    glUseProgram(0);
}

void DeferredPBREffect::setLights(const std::vector<std::shared_ptr<Light>>& lights) {
    std::size_t lightIndex = 0;

    glUseProgram(program);

    glUniform1i(glGetUniformLocation(program, "numLights"), lights.size());

    for (const auto& light : lights) {
        auto lightInfo = light->getLightInfo();

        // position
        std::ostringstream positionLocation;
        std::ostringstream colorLocation;
        std::ostringstream intensityLocation;
        std::ostringstream attenuationLocation;
        std::ostringstream ambientCoefficientLocation;
        std::ostringstream enabledLocation;
        std::ostringstream coneAngleLocation;
        std::ostringstream coneDirectionLocation;

        positionLocation << "lights[" << lightIndex << "].position";
        colorLocation << "lights[" << lightIndex << "].color";
        intensityLocation << "lights[" << lightIndex << "].intensity";
        attenuationLocation << "lights[" << lightIndex << "].attenuation";
        ambientCoefficientLocation << "lights[" << lightIndex << "].ambientCoefficient";
        enabledLocation << "lights[" << lightIndex << "].enabled";
        coneAngleLocation << "lights[" << lightIndex << "].coneAngle";
        coneDirectionLocation << "lights[" << lightIndex << "].coneDirection";

        glUniform4fv(
            glGetUniformLocation(program, positionLocation.str().c_str()),
            1,
            glm::value_ptr(lightInfo.position)
        );

        glUniform3fv(
            glGetUniformLocation(program, colorLocation.str().c_str()),
            1,
            glm::value_ptr(lightInfo.color)
        );

        glUniform1f(
            glGetUniformLocation(program, intensityLocation.str().c_str()),
            lightInfo.intensity
        );

        glUniform1f(
            glGetUniformLocation(program, attenuationLocation.str().c_str()),
            lightInfo.attenuation
        );

        glUniform1f(
            glGetUniformLocation(program, ambientCoefficientLocation.str().c_str()),
            lightInfo.ambientCoefficient
        );

        glUniform1f(
            glGetUniformLocation(program, enabledLocation.str().c_str()),
            lightInfo.enabled ? 1.0f : 0.0f
        );

        glUniform1f(
            glGetUniformLocation(program, coneAngleLocation.str().c_str()),
            lightInfo.coneAngle
        );

        glUniform3fv(
            glGetUniformLocation(program, coneDirectionLocation.str().c_str()),
            1,
            glm::value_ptr(lightInfo.coneDirection)
        );


        lightIndex++;
    }

    glUseProgram(0);
}

void DeferredPBREffect::setViewMatrix(const glm::mat4& viewMatrix) {
    glUseProgram(program);
    auto viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUseProgram(0);
}

void DeferredPBREffect::toggleSSAO(bool value) {
    glUseProgram(program);
    auto ssaoEnabledLocation = glGetUniformLocation(program, "ssaoEnabled");
    glUniform1f(ssaoEnabledLocation, value ? 1.0f : 0.0f);
    glUseProgram(0);
}

void DeferredPBREffect::toggleIBL(bool value) {
    glUseProgram(program);
    auto iblEnabledLocation = glGetUniformLocation(program, "iblEnabled");
    glUniform1f(iblEnabledLocation, value ? 1.0f : 0.0f);
    glUseProgram(0);
}

void DeferredPBREffect::render(
    GLuint vao,
    GLuint ambientOcclusion,
    GLuint diffuseIrradianceMap,
    GLuint prefilteredEnvironmentMap,
    GLuint integratedBRDFMap
) {
    glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the screen object to it
    glBindVertexArray(vao);
    auto deferredProgram = program;
    // use the debug program from the deferred target (just render 1 property)
    glUseProgram(deferredProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, albedoTexture);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, emissiveTexture);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, roughnessAndMetalnessTexture);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, ambientOcclusion);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, diffuseIrradianceMap);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredEnvironmentMap);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, integratedBRDFMap);

    glUniform1i(glGetUniformLocation(deferredProgram, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(deferredProgram, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(deferredProgram, "gAlbedo"), 2);
    glUniform1i(glGetUniformLocation(deferredProgram, "gEmissive"), 3);
    glUniform1i(glGetUniformLocation(deferredProgram, "gRoughnessAndMetalness"), 4);
    glUniform1i(glGetUniformLocation(deferredProgram, "ambientOcclusion"), 5);
    glUniform1i(glGetUniformLocation(deferredProgram, "diffuseIrradianceMap"), 6);
    glUniform1i(glGetUniformLocation(deferredProgram, "prefilteredEnvironmentMap"), 7);
    glUniform1i(glGetUniformLocation(deferredProgram, "integratedBRDFMap"), 8);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}
