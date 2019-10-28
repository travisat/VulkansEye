#include "Model.hpp"
#include "Vulkan.hpp"
#include "helpers.h"

namespace tat
{

void Model::create()
{
    position = config->position;
    rotation = config->rotation;
    scale = config->scale;
    uTessControl.tessLevel = config->tessLevel;
    uTessEval.tessStrength = config->tessStregth;
    uTessEval.tessAlpha = config->tessAlpha;

    material = materials->getMaterial(config->material);
    loadMesh();

    createUniformBuffers();
}

void Model::loadMesh()
{
    loadObj(config->object, vertices, indices);
    vertexSize = static_cast<uint32_t>(vertices.size());
    indexSize = static_cast<uint32_t>(indices.size());
    Trace("Loaded ", config->object, " at ", Timer::systemTime());

    // copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    stagingBuffer.update(vertices);
    vertexBuffer.vulkan = vulkan;
    vertexBuffer.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(vertexBuffer);

    stagingBuffer.update(indices);
    indexBuffer.vulkan = vulkan;
    indexBuffer.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(indexBuffer);
}

void Model::createColorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    std::vector<VkDescriptorSetLayout> layouts(vulkan->swapChainImages.size(), layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    colorSets.resize(vulkan->swapChainImages.size());
    CheckResult(vkAllocateDescriptorSets(vulkan->device, &allocInfo, colorSets.data()));
    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        VkDescriptorBufferInfo tessControlInfo = {};
        tessControlInfo.buffer = tescBuffers[i].buffer;
        tessControlInfo.offset = 0;
        tessControlInfo.range = sizeof(TessControl);

        VkDescriptorBufferInfo tessEvalInfo = {};
        tessEvalInfo.buffer = teseBuffers[i].buffer;
        tessEvalInfo.offset = 0;
        tessEvalInfo.range = sizeof(TessEval);

        VkDescriptorImageInfo dispInfo = {};
        dispInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        dispInfo.imageView = material->displacement.imageView;
        dispInfo.sampler = material->displacement.sampler;

        VkDescriptorBufferInfo lightInfo = {};
        lightInfo.buffer = uniformLights[i].buffer;
        lightInfo.offset = 0;
        lightInfo.range = sizeof(UniformLight);

        VkDescriptorImageInfo shadowInfo = {};
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowInfo.imageView = shadow->imageView;
        shadowInfo.sampler = shadow->sampler;

        VkDescriptorImageInfo diffuseInfo = {};
        diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        diffuseInfo.imageView = material->diffuse.imageView;
        diffuseInfo.sampler = material->diffuse.sampler;

        VkDescriptorImageInfo normalInfo = {};
        normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalInfo.imageView = material->normal.imageView;
        normalInfo.sampler = material->normal.sampler;

        VkDescriptorImageInfo roughnessInfo = {};
        roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        roughnessInfo.imageView = material->roughness.imageView;
        roughnessInfo.sampler = material->roughness.sampler;

        VkDescriptorImageInfo metallicInfo = {};
        metallicInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        metallicInfo.imageView = material->metallic.imageView;
        metallicInfo.sampler = material->metallic.sampler;

        VkDescriptorImageInfo aoInfo = {};
        aoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        aoInfo.imageView = material->ao.imageView;
        aoInfo.sampler = material->ao.sampler;

        std::array<VkWriteDescriptorSet, 10> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = colorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &tessControlInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = colorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &tessEvalInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = colorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &dispInfo;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = colorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &lightInfo;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = colorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &shadowInfo;

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = colorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &diffuseInfo;

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = colorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &normalInfo;

        descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[7].dstSet = colorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[7].descriptorCount = 1;
        descriptorWrites[7].pImageInfo = &roughnessInfo;

        descriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[8].dstSet = colorSets[i];
        descriptorWrites[8].dstBinding = 8;
        descriptorWrites[8].dstArrayElement = 0;
        descriptorWrites[8].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[8].descriptorCount = 1;
        descriptorWrites[8].pImageInfo = &metallicInfo;

        descriptorWrites[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[9].dstSet = colorSets[i];
        descriptorWrites[9].dstBinding = 9;
        descriptorWrites[9].dstArrayElement = 0;
        descriptorWrites[9].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[9].descriptorCount = 1;
        descriptorWrites[9].pImageInfo = &aoInfo;

        vkUpdateDescriptorSets(vulkan->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                               0, nullptr);
    }
}

void Model::createShadowSets(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    std::vector<VkDescriptorSetLayout> layouts(vulkan->swapChainImages.size(), layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    shadowSets.resize(vulkan->swapChainImages.size());
    CheckResult(vkAllocateDescriptorSets(vulkan->device, &allocInfo, shadowSets.data()));
    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        VkDescriptorBufferInfo shadowInfo = {};
        shadowInfo.buffer = shadowBuffers[i].buffer;
        shadowInfo.offset = 0;
        shadowInfo.range = sizeof(UniformShadow);

        VkWriteDescriptorSet shadow{};
        shadow.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        shadow.dstSet = shadowSets[i];
        shadow.dstBinding = 0;
        shadow.dstArrayElement = 0;
        shadow.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        shadow.descriptorCount = 1;
        shadow.pBufferInfo = &shadowInfo;

        std::array<VkWriteDescriptorSet, 1> descriptorWrites = {shadow};

        vkUpdateDescriptorSets(vulkan->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                               0, nullptr);
    }
}

void Model::createUniformBuffers()
{
    uniformLights.resize(vulkan->swapChainImages.size());
    tescBuffers.resize(vulkan->swapChainImages.size());
    teseBuffers.resize(vulkan->swapChainImages.size());
    shadowBuffers.resize(vulkan->swapChainImages.size());

    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        tescBuffers[i].vulkan = vulkan;
        tescBuffers[i].flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        tescBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        tescBuffers[i].resize(sizeof(TessControl));

        teseBuffers[i].vulkan = vulkan;
        teseBuffers[i].flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        teseBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        teseBuffers[i].resize(sizeof(TessEval));

        uniformLights[i].vulkan = vulkan;
        uniformLights[i].flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        uniformLights[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        uniformLights[i].resize(sizeof(UniformLight));

        shadowBuffers[i].vulkan = vulkan;
        shadowBuffers[i].flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        shadowBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        shadowBuffers[i].resize(sizeof(UniformShadow));
    }
}

void Model::loadObj(const std::string &path, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()));

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex = {};
            vertex.position = {(attrib.vertices[3 * index.vertex_index + 0]),
                               (attrib.vertices[3 * index.vertex_index + 1]),
                               (attrib.vertices[3 * index.vertex_index + 2])};
            vertex.UV = {attrib.texcoords[2 * index.texcoord_index + 0],
                         1.0F - attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.normal = {attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
                             attrib.normals[3 * index.normal_index + 2]};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

} // namespace tat