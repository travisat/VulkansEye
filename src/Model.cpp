#include "Model.hpp"

Model::Model(State *state, uint32_t id, glm::vec3 position, glm::vec3 scale, Mesh *mesh, Material *material)
{
    this->state = state;
    this->id = id;
    this->position = position; 
    this->scale = scale;
    this->mesh = mesh;
    this->material = material;
    
}

Model::Model(State *state, uint32_t id, ModelType type, glm::vec3 position, glm::vec3 scale, std::string path){
    this->state = state;
    this->id = id;
    this->position = position; 
    this->scale = scale;

   //TODO create loading other formats
    
}

Model::~Model()
    {
        for (auto buffer : uniformBuffers)
        {
            delete buffer;
        }
        for (auto light : uniformLights)
        {
            delete light;
        }
    }
