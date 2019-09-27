#include "Model.h"


Model::Model(State* state, Mesh *mesh, Material *material, glm::vec3 position){
    this->state = state;
    this->mesh = mesh;
    this->material = material;
    this->position = position; 

    uniformBuffers = {};
    descriptorSets = {};
}

Model::Model(State *state, std::string path, ModelType type, glm::vec3 position)
{
    /// TODO
}

Model::~Model()
    {
        for (auto buffer : uniformBuffers)
        {
            delete buffer;
        }
    }
