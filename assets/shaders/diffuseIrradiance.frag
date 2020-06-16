#version 330 core

out vec3 fragColor;

in vec3 vPosition;

uniform samplerCube environmentMap;

const float PI = 3.1415926535;

void main() {
    vec3 N = normalize(vPosition);

    vec3 irradiance = vec3(0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float numSamples = 0;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {

            vec3 tangent = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangent.x * right + tangent.y * up + tangent.z * N;
            
            vec3 sampleRadiance = texture(environmentMap, normalize(sampleVec)).rgb;

            irradiance += sampleRadiance * cos(theta) * sin(theta);
            numSamples++;
        }
    }

    fragColor = vec3(PI * irradiance / numSamples);
}
