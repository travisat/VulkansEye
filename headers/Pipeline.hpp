#pragma once

#include "Vertex.h"
#include "Vulkan.hpp"

namespace tat
{
class Pipeline
{
  public:
    Vulkan *vulkan;
    VkDescriptorSetLayout descriptorSetLayout;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    ~Pipeline();
    void create();
    void cleanup();
    void loadDefaults();

    void createScenePipeline();
    void createBackdropPipeline();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    VkPipelineShaderStageCreateInfo tescShaderStageInfo = {};
    VkPipelineShaderStageCreateInfo teseShaderStageInfo = {};
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    std::vector<VkDynamicState> dynamicStateEnables = {};
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    VkPipelineTessellationStateCreateInfo tessellationState{};
    VkPipelineViewportStateCreateInfo viewportState = {};
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    VkGraphicsPipelineCreateInfo pipelineInfo = {};

  private:
};
} // namespace tat