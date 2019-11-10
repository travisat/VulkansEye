#pragma once

#include "Vertex.hpp"
#include "Vulkan.hpp"
#include <memory>

namespace tat
{
class Pipeline
{
  public:
    std::shared_ptr<Vulkan> vulkan;
    vk::DescriptorSetLayout descriptorSetLayout;

    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;

    ~Pipeline();
    void create();
    void cleanup();
    void loadDefaults(vk::RenderPass renderPass);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vk::PipelineShaderStageCreateInfo tescShaderStageInfo = {};
    vk::PipelineShaderStageCreateInfo teseShaderStageInfo = {};
    vk::PipelineShaderStageCreateInfo geomShaderStageInfo = {};
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {};
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    std::vector<vk::DynamicState> dynamicStateEnables = {};
    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    vk::PipelineTessellationStateCreateInfo tessellationState{};
    vk::PipelineViewportStateCreateInfo viewportState = {};
    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
    vk::GraphicsPipelineCreateInfo pipelineInfo = {};

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
};
} // namespace tat