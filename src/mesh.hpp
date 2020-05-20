#pragma once

#include <string>

class Mesh {
    public:
        Mesh();
        ~Mesh();

        Mesh(Mesh&& other) = default;
        Mesh(const Mesh& other) = default;

        Mesh& operator=(const Mesh& other) = default;
        Mesh& operator=(Mesh&& other) = default;

        Mesh& fromOBJ(std::string filename);
    private:
};
