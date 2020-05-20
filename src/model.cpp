#include "model.hpp"

#include "material.hpp"
#include "mesh.hpp"

// ctor - Takes in unique_ptr references and moves them.
// Question: Is it better to do the std::move here, or do it when we call the
// ctor in the first place?
Model::Model(std::unique_ptr<Mesh>& mesh, std::unique_ptr<Material>& material) :
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
