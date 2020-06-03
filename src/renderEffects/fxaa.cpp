#include "fxaa.hpp"

#include "gl/shaderUtils.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>

FXAAEffect::FXAAEffect(int w, int h) :
    width(w),
    height(h)
{}

FXAAEffect::~FXAAEffect() {
    // TODO: Free buffers
}

// Must call this AFTER GL/SDL have been initialized
void FXAAEffect::initialize() {
    createProgram();
}

void FXAAEffect::createProgram() {
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

        const float EDGE_THRESHOLD_MIN = 0.0312;
        const float EDGE_THRESHOLD_MAX = 0.125;

        const int ITERATIONS = 12;

        uniform sampler2D input;

        uniform float widthInv;
        uniform float heightInv;

        in vec2 vUv;

        out vec4 fragColor;

        float computeLuma(vec3 color) {
            // note that we have already applied
            // gamma correction in the lighting pass, so don't do it again
            return dot(color, vec3(0.299, 0.587, 0.114));
        }

        float quality(int i) {
            if (i < 1) {
                return 1.5;

            } else if (i < 5) {
                return 2.0;
            } else if (i < 6) {
                return 4.0;
            }
            return 8.0;
        }

        void main() {
            // 1. Compute Luma

            vec3 color = texture(input, vUv).rgb;

            float M = computeLuma(color);
            float D = computeLuma(textureOffset(input, vUv, ivec2(0, -1)).rgb);
            float U = computeLuma(textureOffset(input, vUv, ivec2(0, 1)).rgb);
            float R = computeLuma(textureOffset(input, vUv, ivec2(1, 0)).rgb);
            float L = computeLuma(textureOffset(input, vUv, ivec2(-1, 0)).rgb);

            float lumaMin = min(M, min(D, min(U, min(R, L))));
            float lumaMax = max(M, max(D, max(U, max(R, L))));

            // delta in luma between brightest and darkest surrounding fragment
            float lumaDelta = lumaMax - lumaMin;

            // 2. Edge Detection
            // if not an edge, just return the color
            if (lumaDelta < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX)) {
                fragColor = vec4(color, 1.0);
                return;
            }

            // 3. Get the remaining corners
            float DL = computeLuma(textureOffset(input, vUv, ivec2(-1, -1)).rgb);
            float DR = computeLuma(textureOffset(input, vUv, ivec2(-1, 1)).rgb);
            float UL = computeLuma(textureOffset(input, vUv, ivec2(1, -1)).rgb);
            float UR = computeLuma(textureOffset(input, vUv, ivec2(1, 1)).rgb);

            float DU = D + U;
            float LR = L + R;

            float LCorners = DL + UL;
            float RCorners = DR + UR;
            float UCorners = UL + UR;
            float DCorners = DL + DR;

            // Decide whether the edge is horizontal or vertical
            // by estimating gradient along each axis
            float edgeHorizontal = abs(-2.0 * L + LCorners)
                + abs(-2.0 * M + DU) * 2.0
                + abs(-2.0 * R + RCorners);

            float edgeVertical = abs(-2.0 * U + UCorners)
                + abs(-2.0 * M + LR) * 2.0
                + abs(-2.0 * D + DCorners);

            bool isHorizontal = edgeHorizontal >= edgeVertical;

            float luma1 = isHorizontal ? D : L;
            float luma2 = isHorizontal ? U : R;

            // compute gradients between the two "sides" and the center
            float gradient1 = luma1 - M;
            float gradient2 = luma2 - M;

            // determine which "side" has the edge
            bool useGradient1 = abs(gradient1) >= abs(gradient2);

            float scaledGradient = 0.25 * max(abs(gradient1), abs(gradient2));

            float stepSize = isHorizontal ? heightInv : widthInv;

            float lumaLocalAverage = 0.0;

            // the step size is used to shift the vUv from the center to the location of the edge (between two pixels).
            if (useGradient1) {
                stepSize = -stepSize;
                lumaLocalAverage = 0.5 * (luma1 + M);
            } else {
                lumaLocalAverage = 0.5 * (luma2 + M);
            }

            // shiftedUv represents the location of the edge
            vec2 shiftedUv = vUv;

            if (isHorizontal) {
                shiftedUv.y += stepSize * 0.5;
            } else {
                shiftedUv.x += stepSize * 0.5;
            }

            vec2 offset = isHorizontal ? vec2(widthInv, 0.0) : vec2(0.0, heightInv);

            // "explore" along the length of the edge to find the ends of it.
            vec2 uv1 = shiftedUv - offset;
            vec2 uv2 = shiftedUv + offset;

            float lumaEnd1 = computeLuma(texture(input, uv1).rgb);
            float lumaEnd2 = computeLuma(texture(input, uv2).rgb);
            lumaEnd1 -= lumaLocalAverage;
            lumaEnd2 -= lumaLocalAverage;

            // we've found the end of the edge if the luma delta between
            // the average of our pixels is greater than the local gradient
            bool reached1 = abs(lumaEnd1) >= scaledGradient;
            bool reached2 = abs(lumaEnd2) >= scaledGradient;
            bool reachedBoth = reached1 && reached2;

            if (!reached1) {
                uv1 -= offset;
            }
            if (!reached2) {
                uv2 += offset;
            }

            if (!reachedBoth) {
                // continue to iterate out until we find the end of the edge
                for (int i = 2; i < ITERATIONS; i++) {
                    if (!reached1) {
                        lumaEnd1 = computeLuma(texture(input, uv1).rgb);
                        lumaEnd1 = lumaEnd1 - lumaLocalAverage;
                    }

                    if (!reached2) {
                        lumaEnd2 = computeLuma(texture(input, uv2).rgb);
                        lumaEnd2 = lumaEnd2 - lumaLocalAverage;
                    }

                    reached1 = abs(lumaEnd1) >= scaledGradient;
                    reached2 = abs(lumaEnd2) >= scaledGradient;
                    reachedBoth = reached1 && reached2;
                    if (!reached1) {
                        uv1 -= offset * quality(i);
                    }

                    if (!reached2) {
                        uv2 += offset * quality(i);
                    }

                    // stop exploring once we've found both ends of the edge
                    if (reachedBoth) {
                        break;
                    }
                }
            }

            float distance1 = isHorizontal ? vUv.x - uv1.x : vUv.y - uv1.y;
            float distance2 = isHorizontal ? uv2.x - vUv.x : uv2.y - vUv.y;

            bool isDirection1 = distance1 < distance2;
            float distanceFinal = min(distance1, distance2);

            float edgeThickness = (distance1 + distance2);

            float pixelOffset = -distanceFinal / edgeThickness + 0.5;

            bool isLumaCenterSmaller = M < lumaLocalAverage;

            bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

            float finalOffset = correctVariation ? pixelOffset : 0.0;

            // TODO: Subpixel AA

            vec2 finalUv = vUv;
            if (isHorizontal) {
                finalUv.y += finalOffset * stepSize;
            } else {
                finalUv.x += finalOffset * stepSize;
            }

            vec3 finalColor = texture(input, finalUv).rgb;
            fragColor = vec4(finalColor, 1.0);
        }
    )";

    program = ShaderUtils::compile(vertexShader, fragmentShader);

    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "widthInv"), 1.0f / static_cast<float>(width));
    glUniform1f(glGetUniformLocation(program, "heightInv"), 1.0f / static_cast<float>(height));

    glUseProgram(0);
}

void FXAAEffect::render(GLuint vao, GLuint input) {
    // FXAA is the final pass and is
    // rendered directly to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    // bind the position texture to texture slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input);
    glUniform1i(glGetUniformLocation(program, "input"), 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
}
