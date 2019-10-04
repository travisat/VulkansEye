#include "Model.hpp"

Model::Model(tat::Vulkan *vulkan, const ModelConfig &config)
{
    this->vulkan = vulkan;
    id = config.id;
    position = config.position;
    scale = config.scale;
    
    //TODO implement using path to load model

    material.vulkan = vulkan;
    material.loadConfig(config.materialConfig);
    mesh.loadConfig(config.meshConfig);
        
}

void Model::draw(const VkCommandBuffer &commandBuffer)
{
            vkCmdDrawIndexed(commandBuffer, mesh.indexSize, 1, mesh.indexOffset, mesh.vertexOffset, 0);
}