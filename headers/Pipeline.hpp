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
    void loadDefaults(VkRenderPass renderPass);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    VkPipelineShaderStageCreateInfo tescShaderStageInfo = {};
    VkPipelineShaderStageCreateInfo teseShaderStageInfo = {};
    VkPipelineShaderStageCreateInfo geomShaderStageInfo = {};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
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