#pragma once

#include <memory>

// Forward declare dependencies to reduce compilation-unit dependencies
class Material;
class Mesh;

class Model {
    public:
        // ctor. Takes in unique pointer references.
        Model(std::unique_ptr<Mesh>& mesh, std::unique_ptr<Material>& material);
        // Move ctor
        Model(Model&& other);
        // Move assignment operator
        Model& operator=(Model&& other);

        // Explicitly delete these as we cannot trivially
        // copy a class which has unique pointers
        // Alternatively, we could do a deep copy on mesh and material
        // But there's no way to simply copy the pointers, unless we use shared_ptrs.
        Model(const Model& other) = delete;
        Model& operator=(const Model& other) = delete;

        // Cannot use = default; as the compiler will complain about calling the destructor
        // of incomplete types
        // Therefore we have to define this in the .cpp file (even if it does nothing)
        ~Model();
    private:
        std::unique_ptr<Mesh> mesh;
        std::unique_ptr<Material> material;
};
