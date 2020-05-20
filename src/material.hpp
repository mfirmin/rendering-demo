#pragma once

#include <GL/glew.h>

#include <string>

class Material {
    public:
        Material();
        ~Material();

        Material(Material&& other) = default;
        Material(const Material& other) = default;

        Material& operator=(const Material& other) = default;
        Material& operator=(Material&& other) = default;
    private:
};
