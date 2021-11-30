#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

// Forward declare dependencies to reduce compilation-unit dependencies
class PointLight;
class Mesh;
class Model;

/**
 * A lamp consists of a mesh with an emissive material + a point-light
 * The mesh is passed into the constructor, and the model and material
 * are created and stored by the lamp
 **/
class Lamp {
    public:
        // ctor. Takes in rvalue pointer references.
        Lamp(
            std::shared_ptr<Mesh> mesh,
            glm::vec3 position,
            glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
            float intensity = 1.0f
        );
        // Move ctor
        Lamp(Lamp&& other) = default;
        // Move assignment operator
        Lamp& operator=(Lamp&& other) = default;

        Lamp(const Lamp& other) = default;
        Lamp& operator=(const Lamp& other) = default;

        ~Lamp() = default;

        void toggle();

        void setColor(glm::vec3 c);
        void setIntensity(float i);
        void setPosition(glm::vec3 p);
        void setRotation(glm::vec3 r);
        void setScale(glm::vec3 s);
        void setScale(float s);

        std::shared_ptr<Model> getModel() {
            return model;
        }

        std::shared_ptr<PointLight> getLight() {
            return light;
        }
    private:
        // Meshes can be shared between models
        std::shared_ptr<Model> model = nullptr;
        // Light
        std::shared_ptr<PointLight> light = nullptr;

        bool active = true;
};
