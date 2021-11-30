#pragma once

#include <GL/glew.h>

#include <vector>


class GLObject {
    public:
        GLObject();
        GLObject(std::vector<float>&& vs, std::vector<float>&& ns);
        ~GLObject();

        GLObject(GLObject&& other) = default;
        GLObject(const GLObject& other) = default;

        GLObject& operator=(const GLObject& other) = default;
        GLObject& operator=(GLObject&& other) = default;

        void setVertices(std::vector<float>&& vertices);
        void setNormals(std::vector<float>&& normals);

        GLuint getVertexArrayObject() const {
            return vertexArrayObject;
        }

        uint32_t getVertexCount() const {
            return vertexCount;
        }
    private:
        uint32_t vertexCount = 0;

        GLuint vertexArrayObject = 0;
        GLuint vertexBuffer = 0;
        GLuint normalBuffer = 0;

        std::vector<float> vertices = {};
        std::vector<float> normals = {};
        // std::vector<float> uvs;

        void createVertexArrayObject();
};
