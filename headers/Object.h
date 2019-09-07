#include "Material.h"
#include "Model.h"

struct Object
{
    uint32_t modelIndex;
    uint32_t materialIndex;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VmaAllocation> uniformBufferMemory;
    std::vector<VkDescriptorSet> descriptorSets;

    double xpos;
    double ypos;
    double zpos;
};