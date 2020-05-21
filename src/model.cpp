#include "model.hpp"

#include "material.hpp"
#include "mesh.hpp"

#include <GL/glew.h>

// ctor - Takes in unique_ptr references and moves them.
// Question: Is it better to do the std::move here, or do it when we call the
// ctor in the first place?
Model::Model(std::unique_ptr<Mesh>&& mesh, std::unique_ptr<Material>&& material) :
    // std::move is still required here, otherwise it would just be a copy, defeating the whole point
    // of passing mesh and material by rvalue ref in the first place
    mesh(std::move(mesh)),
    material(std::move(material))
{}

Model::~Model() {}

Model::Model(Model&& other) {
    mesh = std::move(other.mesh);
    material = std::move(other.material);
}

Model& Model::operator=(Model&& other) {
    mesh = std::move(other.mesh);
    material = std::move(other.material);

    return *this;
}

void Model::setLights(const std::vector<std::unique_ptr<Light>>& lights) {
    material->setLights(lights);
}

void Model::setProjectionAndViewMatrices(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
    material->setProjectionAndViewMatrices(projectionMatrix, viewMatrix);
}

void Model::draw() const {
    glUseProgram(material->getProgram());

    glBindVertexArray(mesh->getVertexArrayObject());
    glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());

    glUseProgram(0);
}
