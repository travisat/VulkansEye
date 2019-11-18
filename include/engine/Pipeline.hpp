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
    void create();
    void destroy();
    void loadDefaults(vk::RenderPass renderPass);

    vk::Pipeline pipeline = nullptr;
    vk::PipelineLayout pipelineLayout = nullptr;

    vk::DescriptorSetLayout *descriptorSetLayout = nullptr;

    vk::ShaderModule vertShader = nullptr;
    vk::ShaderModule fragShader = nullptr;
    vk::ShaderModule geomShader = nullptr;
    vk::ShaderModule tescShader = nullptr;
    vk::ShaderModule teseShader = nullptr;

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