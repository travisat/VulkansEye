#include "Pipeline.hpp"
#include "helpers.h"
#include "vulkan/vulkan.hpp"

namespace tat
{

Pipeline::~Pipeline()
{
    cleanup();
}

void Pipeline::create()
{
    pipelineLayout = vulkan->device.createPipelineLayout(pipelineLayoutInfo);
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipeline = vulkan->device.createGraphicsPipeline(nullptr, pipelineInfo); // TODO(travis) create pipeline cache
}

void Pipeline::cleanup()
{
    if (vertShaderStageInfo.module)
    {
        vulkan->device.destroyShaderModule(vertShaderStageInfo.module);
    }
    if (fragShaderStageInfo.module)
    {
        vulkan->device.destroyShaderModule(fragShaderStageInfo.module);
    }
    if (tescShaderStageInfo.module)
    {
        vulkan->device.destroyShaderModule(tescShaderStageInfo.module);
    }
    if (teseShaderStageInfo.module)
    {
        vulkan->device.destroyShaderModule(teseShaderStageInfo.module);
    }
    if (geomShaderStageInfo.module)
    {
        vulkan->device.destroyShaderModule(geomShaderStageInfo.module);
    }

    if (pipeline)
    {
        vulkan->device.destroyPipeline(pipeline);
    }
    if (pipelineLayout)
    {
        vulkan->device.destroyPipelineLayout(pipelineLayout);
    }
}

void Pipeline::loadDefaults(vk::RenderPass renderPass)
{
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.pName = "main";

    tescShaderStageInfo.stage = vk::ShaderStageFlagBits::eTessellationControl;
    tescShaderStageInfo.pName = "main";

    teseShaderStageInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
    teseShaderStageInfo.pName = "main";

    geomShaderStageInfo.stage = vk::ShaderStageFlagBits::eGeometry;
    geomShaderStageInfo.pName = "main";

    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.pName = "main";

    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;

    tessellationState.patchControlPoints = 3;

    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eLineWidth};

    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0F;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vulkan->msaaSamples;

    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0F;
    colorBlending.blendConstants[1] = 0.0F;
    colorBlending.blendConstants[2] = 0.0F;
    colorBlending.blendConstants[3] = 0.0F;

    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;
}

} // namespace tat