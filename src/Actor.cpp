#include "Actor.hpp"
#include "helpers.h"

namespace tat {

void Actor::create() {
  // TODO implement using path to load model
  name = config->name;
  position = config->position;
  rotation = config->rotation;
  scale = config->scale;

  model.vulkan = vulkan;
  model.config = &config->modelConfig;
  model.create();

  // copy buffers to gpu only memory
  Buffer stagingBuffer{};
  stagingBuffer.vulkan = vulkan;
  stagingBuffer.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  stagingBuffer.update(model.vertices);
  vertexBuffer.vulkan = vulkan;
  vertexBuffer.flags =
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  vertexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
  stagingBuffer.copyTo(vertexBuffer);

  stagingBuffer.update(model.indices);
  indexBuffer.vulkan = vulkan;
  indexBuffer.flags =
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  indexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
  stagingBuffer.copyTo(indexBuffer);
}

void Actor::createDescriptorSets(VkDescriptorPool descriptorPool,
                                 VkDescriptorSetLayout descriptorSetLayout) {
  std::vector<VkDescriptorSetLayout> layouts(vulkan->swapChainImages.size(),
                                             descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount =
      static_cast<uint32_t>(vulkan->swapChainImages.size());
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(vulkan->swapChainImages.size());
  CheckResult(vkAllocateDescriptorSets(vulkan->device, &allocInfo,
                                       descriptorSets.data()));
  for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i) {
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
    dispInfo.imageView = model.displacement.imageView;
    dispInfo.sampler = model.dispSampler;

    VkDescriptorBufferInfo lightInfo = {};
    lightInfo.buffer = uniformLights[i].buffer;
    lightInfo.offset = 0;
    lightInfo.range = sizeof(UniformLight);

    VkDescriptorImageInfo diffuseInfo = {};
    diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    diffuseInfo.imageView = model.diffuse.imageView;
    diffuseInfo.sampler = model.diffuseSampler;

    VkDescriptorImageInfo normalInfo = {};
    normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    normalInfo.imageView = model.normal.imageView;
    normalInfo.sampler = model.normalSampler;

    VkDescriptorImageInfo roughnessInfo = {};
    roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    roughnessInfo.imageView = model.roughness.imageView;
    roughnessInfo.sampler = model.roughnessSampler;

    VkDescriptorImageInfo metallicInfo = {};
    metallicInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    metallicInfo.imageView = model.metallic.imageView;
    metallicInfo.sampler = model.metallicSampler;

    VkDescriptorImageInfo aoInfo = {};
    aoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    aoInfo.imageView = model.ao.imageView;
    aoInfo.sampler = model.aoSampler;

    std::array<VkWriteDescriptorSet, 9> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &tessControlInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &tessEvalInfo;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = descriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &dispInfo;

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = descriptorSets[i];
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &lightInfo;

    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[4].dstSet = descriptorSets[i];
    descriptorWrites[4].dstBinding = 4;
    descriptorWrites[4].dstArrayElement = 0;
    descriptorWrites[4].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[4].descriptorCount = 1;
    descriptorWrites[4].pImageInfo = &diffuseInfo;

    descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[5].dstSet = descriptorSets[i];
    descriptorWrites[5].dstBinding = 5;
    descriptorWrites[5].dstArrayElement = 0;
    descriptorWrites[5].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[5].descriptorCount = 1;
    descriptorWrites[5].pImageInfo = &normalInfo;

    descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[6].dstSet = descriptorSets[i];
    descriptorWrites[6].dstBinding = 6;
    descriptorWrites[6].dstArrayElement = 0;
    descriptorWrites[6].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[6].descriptorCount = 1;
    descriptorWrites[6].pImageInfo = &roughnessInfo;

    descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[7].dstSet = descriptorSets[i];
    descriptorWrites[7].dstBinding = 7;
    descriptorWrites[7].dstArrayElement = 0;
    descriptorWrites[7].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[7].descriptorCount = 1;
    descriptorWrites[7].pImageInfo = &metallicInfo;

    descriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[8].dstSet = descriptorSets[i];
    descriptorWrites[8].dstBinding = 8;
    descriptorWrites[8].dstArrayElement = 0;
    descriptorWrites[8].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[8].descriptorCount = 1;
    descriptorWrites[8].pImageInfo = &aoInfo;

    vkUpdateDescriptorSets(vulkan->device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void Actor::createUniformBuffers() {
  uniformLights.resize(vulkan->swapChainImages.size());
  tescBuffers.resize(vulkan->swapChainImages.size());
  teseBuffers.resize(vulkan->swapChainImages.size());

  for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i) {
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
  }
}

} // namespace tat