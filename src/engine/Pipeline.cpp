#include "engine/Pipeline.hpp"
#include "State.hpp"
#include <spdlog/spdlog.h>

namespace tat
{

void Pipeline::create()
{
    auto &engine = State::instance().engine;
    pipelineLayout = engine.device.create(pipelineLayoutInfo);
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipeline = engine.device.create(pipelineInfo, engine.pipelineCache.pipelineCache);
}

void Pipeline::destroy()
{
    auto &device = State::instance().engine.device;
    if (pipeline)
    {
        device.destroy(pipeline);
    }
    if (pipelineLayout)
    {
        device.destroy(pipelineLayout);
    }
    if (fragShader)
    {
        device.destroy(fragShader);
    }
    if (vertShader)
    {
        device.destroy(vertShader);
    }
    if (geomShader)
    {
        device.destroy(geomShader);
    }
    if (tescShader)
    {
        device.destroy(tescShader);
    }
    if (teseShader)
    {
        device.destroy(teseShader);
    }
}

void Pipeline::loadDefaults(vk::RenderPass renderPass)
{
    auto &engine = State::instance().engine;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.module = vertShader;

    tescShaderStageInfo.stage = vk::ShaderStageFlagBits::eTessellationControl;
    tescShaderStageInfo.pName = "main";
    tescShaderStageInfo.module = tescShader;

    teseShaderStageInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
    teseShaderStageInfo.pName = "main";
    teseShaderStageInfo.module = teseShader;

    geomShaderStageInfo.stage = vk::ShaderStageFlagBits::eGeometry;
    geomShaderStageInfo.pName = "main";
    geomShaderStageInfo.module = geomShader;

    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.module = fragShader;

    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;

    tessellationState.patchControlPoints = 3;

    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eLineWidth};

    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = dynamicStateEnables.size();

    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0F;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = engine.physicalDevice.msaaSamples;

    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.F;
    colorBlending.blendConstants[1] = 0.F;
    colorBlending.blendConstants[2] = 0.F;
    colorBlending.blendConstants[3] = 0.F;

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