#pragma once
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

class Pipeline
{
  public:
    vk::DescriptorSetLayout descriptorSetLayout;

    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout pipelineLayout;

    vk::UniqueShaderModule vertShader;
    vk::UniqueShaderModule fragShader;
    vk::UniqueShaderModule geomShader;
    vk::UniqueShaderModule tescShader;
    vk::UniqueShaderModule teseShader;

    void create();
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
};

} // namespace tat