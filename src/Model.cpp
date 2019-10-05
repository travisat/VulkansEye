#include "Model.hpp"
#include "macros.h"

Model::Model(tat::Vulkan *vulkan, const ModelConfig &config)
{
    this->vulkan = vulkan;
    id = config.id;
    name = config.name;
    position = config.position;
    scale = config.scale;
    
    //TODO implement using path to load model

    material.vulkan = vulkan;
    material.loadConfig(config.materialConfig);
    mesh.loadConfig(config.meshConfig);
        
}

Model::~Model()
{
    Trace("Destroyed ", name, " at ", Timer::systemTime());
}