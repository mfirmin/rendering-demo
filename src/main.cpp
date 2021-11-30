#include "scene.hpp"

#include <GL/glew.h>
#include <memory>
#include <stdlib.h>

const uint32_t DEFAULT_WIDTH = 1600;
const uint32_t DEFAULT_HEIGHT = 900;

int main(int argc, char* argv[]) {
    auto width = DEFAULT_WIDTH;
    auto height = DEFAULT_HEIGHT;

    if (argc == 3) {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
    }

    Scene scene(width, height);

    scene.initialize();

    scene.go();
}
