#version 330 core

out vec2 fragColor;

in vec2 vUv;

uniform float roughness;

const float PI = 3.1415926535;

float radicalInverseVDC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), radicalInverseVDC(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 v1 = normalize(cross(up, N));
    vec3 v2 = cross(N, v1);

    vec3 sampleVec = v1 * H.x + v2 * H.y + N * H.z;

    return normalize(sampleVec);
}

float geometrySchlick(vec3 N, vec3 V, float k) {
    float nDotV = max(dot(N, V), 0.0);

    return nDotV / (nDotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float r) {
    float k = (r * r) / 2.0;
    return geometrySchlick(N, V, k) * geometrySchlick(N, L, k);
}

vec2 integrateBRDF(float nDotV, float roughness) {
    vec3 V;
    V.x = sqrt(1.0 - nDotV * nDotV);
    V.y = 0.0;
    V.z = nDotV;

    float A = 0.0;
    float B = 0.0;

    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;

    for (uint i = 0u; i < SAMPLE_COUNT; i++) {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H = importanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float nDotL = max(L.z, 0.0);
        float nDotH = max(H.z, 0.0);
        float vDotH = max(dot(V, H), 0.0);

        if (nDotL > 0.0) {
            float G = geometrySmith(N, V, L, roughness);
            float G_Vis = (G * vDotH) / (nDotH * nDotV);
            float Fc = pow(1.0 - vDotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);

    return vec2(A, B);
}

void main() {
    fragColor = integrateBRDF(vUv.x, vUv.y);
}
