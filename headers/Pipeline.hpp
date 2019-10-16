#pragma once

#include "Vertex.h"
#include "Vulkan.hpp"


namespace tat {
class Pipeline {
public:
  Vulkan *vulkan;
  VkDescriptorSetLayout descriptorSetLayout;
  std::string vertShaderPath;
  std::string fragShaderPath;
  std::string tescShaderPath;
  std::string teseShaderPath;

  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;

  ~Pipeline();
  void cleanup();

  void createScenePipeline();
  void createBackdropPipeline();

private:
  void createDefaultPipeline();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  VkPipelineShaderStageCreateInfo tessControlShaderStageInfo = {};
  VkPipelineShaderStageCreateInfo tessEvalShaderStageInfo = {};
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
};
} // namespace tat