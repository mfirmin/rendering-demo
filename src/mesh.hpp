#pragma once

#include "gl/glObject.hpp"

#include <string>

class Mesh {
    public:
        Mesh();
        ~Mesh() = default;

        Mesh(Mesh&& other) = default;
        Mesh(const Mesh& other) = default;

        Mesh& operator=(const Mesh& other) = default;
        Mesh& operator=(Mesh&& other) = default;

        Mesh& fromOBJ(std::string filename);

        GLuint getVertexArrayObject() {
            return vertexArrayObject.getVertexArrayObject();
        }

        uint16_t getVertexCount() {
            return vertexArrayObject.getVertexCount();
        }
    private:
        GLObject vertexArrayObject = {};
};
