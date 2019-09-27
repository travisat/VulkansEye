#include "Model.h"

Model::Model(State *state, uint32_t id, glm::vec3 position, Mesh *mesh, Material *material)
{
    this->state = state;
    this->id = id;
    this->position = position; 
    this->mesh = mesh;
    this->material = material;
    
}

Model::Model(State *state, uint32_t id, ModelType type, glm::vec3 position, std::string path){
    this->state = state;
    this->id = id;
    this->position = position; 

   //TODO create loading other formats
    
}

Model::~Model()
    {
        for (auto buffer : uniformBuffers)
        {
            delete buffer;
        }
    }
