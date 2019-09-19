#include "Model.h"


Model::Model(State* state, Mesh *mesh, Material *material){
    this->state = state;
    this->mesh = mesh;
    this->material = material;
    xpos = 0;
    ypos = 0;
    zpos = 0;
}

Model::~Model()
    {
        for (auto buffer : uniformBuffers)
        {
            delete buffer;
        }
    }
