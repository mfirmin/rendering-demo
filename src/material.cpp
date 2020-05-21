#include "material.hpp"

#include "gl/shaderUtils.hpp"

#include "light/light.hpp"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <sstream>

Material::Material(glm::vec3 color) {
    const GLchar* vertexShaderSource[] = {
        R"(
        #version 330
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;
        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;

        out vec3 vNormalEyespace;
        out vec4 vPositionEyespace;

        void main() {
            vNormalEyespace = (transpose(inverse(viewMatrix * modelMatrix)) * vec4(normal, 0.0)).xyz;
            vPositionEyespace = viewMatrix * modelMatrix * vec4(position, 1.0);

            gl_Position = projectionMatrix * vPositionEyespace;
        }
        )"
    };

    const GLchar* fragmentShaderSource[] = {
        R"(
        #version 330

        #define MAX_LIGHTS 10

        out vec4 fColor;

        uniform mat4 viewMatrix;

        uniform vec3 color;
        uniform vec3 specularColor;
        uniform float shininess;

        in vec3 vNormalEyespace;
        in vec4 vPositionEyespace;

        uniform int numLights;
        uniform struct Light {
            vec4 position;
            vec3 color;
            float ambientCoefficient;
            float attenuation;
            // spotlight only
            float coneAngle;
            vec3 coneDirection;
        } lights[MAX_LIGHTS];

        vec3 illuminate(vec3 inColor, vec3 P, vec3 N, vec3 E) {
            vec3 outColor = vec3(0.0);

            for (int i = 0; i < numLights; i++) {
                Light light = lights[i];
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
                // TODO: Spotlights

                vec3 ambient = light.ambientCoefficient * inColor * light.color;

                float diffuseCoefficient = max(0.0, dot(N, L));
                vec3 diffuse = diffuseCoefficient * inColor * light.color;

                float specularCoefficient = 0.0;

                // TODO: FIXME
                vec3 specular = vec3(0.0);

                if (diffuseCoefficient > 0.0) {
                    specularCoefficient = pow(
                        max(
                            0.0,
                            dot(
                                E,
                                reflect(-L, N)
                            )
                        ),
                        shininess
                    );
                }

                // TODO: Shadows

                outColor += ambient + attenuation * (diffuse + specular);
            }

            // TODO: Gamma correction
            return outColor;
        }

        void main() {
            vec3 N = normalize(vNormalEyespace);
            vec3 E = normalize(-vPositionEyespace.xyz);

            fColor = vec4(illuminate(color, vPositionEyespace.xyz, N, E), 1.0);
        }
    )"
    };

    program = ShaderUtils::compile(vertexShaderSource, fragmentShaderSource);

    if (program == 0) {
        return;
    }

    glUseProgram(program);
    auto projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    auto modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");
    auto colorLocation = glGetUniformLocation(program, "color");
    auto specularColorLocation = glGetUniformLocation(program, "specularColor");
    auto shininessLocation = glGetUniformLocation(program, "shininess");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glUniform3fv(specularColorLocation, 1, glm::value_ptr(color));
    glUniform1f(shininessLocation, 0.0f);
    glUseProgram(0);
}

Material::~Material() {}

void Material::setColor(glm::vec3 color) {
    glUseProgram(program);
    auto colorLocation = glGetUniformLocation(program, "color");
    auto specularColorLocation = glGetUniformLocation(program, "specularColor");
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glUniform3fv(specularColorLocation, 1, glm::value_ptr(color));
    glUseProgram(0);
}

void Material::setLights(const std::vector<std::unique_ptr<Light>>& lights) {
    std::size_t lightIndex = 0;

    glUseProgram(program);

    glUniform1i(glGetUniformLocation(program, "numLights"), lights.size());

    for (const auto& light : lights) {
        auto lightInfo = light->getLightInfo();

        // position
        std::ostringstream positionLocation;
        std::ostringstream colorLocation;
        std::ostringstream attenuationLocation;
        std::ostringstream ambientCoefficientLocation;
        std::ostringstream coneAngleLocation;
        std::ostringstream coneDirectionLocation;

        positionLocation << "lights[" << lightIndex << "].position";
        colorLocation << "lights[" << lightIndex << "].color";
        attenuationLocation << "lights[" << lightIndex << "].attenuation";
        ambientCoefficientLocation << "lights[" << lightIndex << "].ambientCoefficient";
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
            glGetUniformLocation(program, attenuationLocation.str().c_str()),
            lightInfo.attenuation
        );

        glUniform1f(
            glGetUniformLocation(program, ambientCoefficientLocation.str().c_str()),
            lightInfo.ambientCoefficient
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

void Material::setProjectionAndViewMatrices(
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix
) {
    glUseProgram(program);
    auto projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUseProgram(0);
}

void Material::setMatrices(
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
) {
    glUseProgram(program);
    auto projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    auto viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    auto modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUseProgram(0);
}
