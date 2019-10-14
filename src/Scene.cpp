#include "Scene.hpp"
#include "helpers.h"

Scene::~Scene()
{
    vkDestroyPipeline(vulkan->device, pipeline, nullptr);
    vkDestroyPipelineLayout(vulkan->device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(vulkan->device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Scene::create()
{
    createLights();
    createActors();
    createBackdrop();
    createStage();

    createDescriptorPool();
    createDescriptorSetLayouts();
    createPipelineLayout();
    createPipeline();
    createUniformBuffers();
    createDescriptorSets();
}

void Scene::createBackdrop()
{

    backdrop.vulkan = vulkan;
    backdrop.player = player;
    backdrop.name = "Backdrop";
    backdrop.path = config->backdrop;
    backdrop.create();
}
void Scene::createStage()
{
    stage.vulkan = vulkan;
    stage.config = &config->stageConfig;
    stage.create();
}

void Scene::createLights()
{
    pointLights.resize(numLights);
    for (auto &lightConfig : config->pointLights)
    {
        pointLights[lightConfig.index].config = &lightConfig;
        pointLights[lightConfig.index].load();
    }
}

void Scene::createActors()
{
    actors.resize(config->actors.size());
    for (auto &actorConfig : config->actors)
    {
        actors[actorConfig.index].config = &actorConfig;
        actors[actorConfig.index].vulkan = vulkan;
        actors[actorConfig.index].create();
    }
}

void Scene::cleanup()
{
    backdrop.cleanup();
    vkDestroyPipeline(vulkan->device, pipeline, nullptr);
    vkDestroyPipelineLayout(vulkan->device, pipelineLayout, nullptr);
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Scene::recreate()
{
    player->updateAspectRatio((float)vulkan->width, (float)vulkan->height);
    backdrop.recreate();
    createPipelineLayout();
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    backdrop.draw(commandBuffer, currentImage);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &stage.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, stage.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &stage.descriptorSets[currentImage], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, stage.model.indexSize, 1, 0, 0, 0);

    for (auto &actor : actors)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &actor.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, actor.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &actor.descriptorSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, actor.model.indexSize, 1, 0, 0, 0);
    }
}

void Scene::createUniformBuffers()
{
    stage.createUniformBuffers();

    for (auto &actor : actors)
    {
        actor.createUniformBuffers();
    }
}

void Scene::updateUniformBuffer(uint32_t currentImage)
{
    backdrop.updateUniformBuffer(currentImage);

    uBuffer.model = glm::mat4(1.0f);
    uBuffer.projection = player->perspective;
    uBuffer.view = player->view;

    for (int32_t i = 0; i < numLights; ++i)
    {
        uLight.light[i].position = pointLights[i].light.position;
        uLight.light[i].temperature = pointLights[i].light.temperature;
        uLight.light[i].lumens = pointLights[i].light.lumens;
    }

    uLight.light[0].position = player->position * -1.0f;

    stage.uniformBuffers[currentImage].update(&uBuffer, sizeof(UniformBuffer));
    stage.uniformLights[currentImage].update(&uLight, sizeof(UniformLight));

    for (auto &actor : actors)
    {
        uBuffer.model = glm::translate(glm::mat4(1.0f), actor.position);

        actor.uniformBuffers[currentImage].update(&uBuffer, sizeof(UniformBuffer));
        actor.uniformLights[currentImage].update(&uLight, sizeof(UniformLight));
    }
}

void Scene::createDescriptorPool()
{
    uint32_t numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (numUniformBuffers() + numUniformLights() * numLights) * numSwapChainImages;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = numImageSamplers() * numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = (numUniformBuffers() + numUniformLights() + numImageSamplers()) * numSwapChainImages;

    CheckResult(vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr, &descriptorPool));
}

void Scene::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uBufferLayoutBinding = {};
    uBufferLayoutBinding.binding = 0;
    uBufferLayoutBinding.descriptorCount = 1;
    uBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uBufferLayoutBinding.pImmutableSamplers = nullptr;
    uBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding uLightLayoutBinding = {};
    uLightLayoutBinding.binding = 1;
    uLightLayoutBinding.descriptorCount = 1;
    uLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uLightLayoutBinding.pImmutableSamplers = nullptr;
    uLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding diffuseLayoutBinding = {};
    diffuseLayoutBinding.binding = 2;
    diffuseLayoutBinding.descriptorCount = 1;
    diffuseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    diffuseLayoutBinding.pImmutableSamplers = nullptr;
    diffuseLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalLayoutBinding = {};
    normalLayoutBinding.binding = 3;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalLayoutBinding.pImmutableSamplers = nullptr;
    normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding roughnessLayoutBinding = {};
    roughnessLayoutBinding.binding = 4;
    roughnessLayoutBinding.descriptorCount = 1;
    roughnessLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    roughnessLayoutBinding.pImmutableSamplers = nullptr;
    roughnessLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding metallicLayoutBinding = {};
    metallicLayoutBinding.binding = 5;
    metallicLayoutBinding.descriptorCount = 1;
    metallicLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    metallicLayoutBinding.pImmutableSamplers = nullptr;
    metallicLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding aoLayoutBinding = {};
    aoLayoutBinding.binding = 6;
    aoLayoutBinding.descriptorCount = 1;
    aoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aoLayoutBinding.pImmutableSamplers = nullptr;
    aoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 7> bindings = {uBufferLayoutBinding, uLightLayoutBinding, diffuseLayoutBinding, normalLayoutBinding, roughnessLayoutBinding, metallicLayoutBinding, aoLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr, &descriptorSetLayout));
}

void Scene::createDescriptorSets()
{
    stage.createDescriptorSets(descriptorPool, descriptorSetLayout);

    for (auto &actor : actors)
    {
        actor.createDescriptorSets(descriptorPool, descriptorSetLayout);
    }
}

void Scene::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    CheckResult(vkCreatePipelineLayout(vulkan->device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
}

void Scene::createPipeline()
{
    auto vertShaderCode = tat::readFile("resources/shaders/scene.vert.spv");
    auto fragShaderCode = tat::readFile("resources/shaders/scene.frag.spv");

    VkShaderModule vertShaderModule = vulkan->createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = vulkan->createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

    auto attributeDescrption = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vulkan->msaaSamples;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = 0xf;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = vulkan->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    CheckResult(vkCreateGraphicsPipelines(vulkan->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

    vkDestroyShaderModule(vulkan->device, fragShaderModule, nullptr);
    vkDestroyShaderModule(vulkan->device, vertShaderModule, nullptr);
}