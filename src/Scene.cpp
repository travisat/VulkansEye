#include "Scene.h"

Scene::Scene(State *state, Config &config)
{
    this->state = state;
    this->config = &config;
};

Scene::~Scene()
{
    delete vertexBuffer;
    delete indexBuffer;

    for (auto object : objects) { delete object;};
    for (auto model : models) { delete model;};
    for (auto material : materials ) { delete material;};
    delete skybox;
}

void Scene::initScene()
{
    initModels();
    initMaterials();
    initObjects();
    loadObjects();
    loadSkybox();
    createPipelines();
    createUniformBuffers();
};

void Scene::initModels()
{
    uint32_t i = 0;
    for (auto &path : config->modelPaths)
    {
        Model *model = new Model(path);
        model->id = i;
        i++;
        models.push_back(model);
    }
}

void Scene::initMaterials()
{
    uint32_t i = 0;
    for (auto &path : config->materialPaths)
    {
        Material *material = new Material(state, path);
        material->id = i;
        i++;
        materials.push_back(material);
    }
}

void Scene::initObjects()
{
    uint32_t i = 0;
    for (auto &index : config->objectIndices)
    {
        Object *object = new Object;
        for (uint32_t i = 0; i < models.size(); i++)
        {
            if (models[i]->id == index[0])
            {
                object->modelIndex = i;
            }
        }
        for (uint32_t i = 0; i < materials.size(); i++)
        {
            if (materials[i]->id == index[1])
            {
                object->materialIndex = i;
            }
        }

        std::array<uint32_t, 3> positions = config->objectPositions[i];

        //convert from 0-100 to -1.0 to 1.0
        object->xpos = (static_cast<double>(positions[0]) / 50.0f) - 1.0f;
        object->ypos = (static_cast<double>(positions[1]) / 50.0f) - 1.0f;
        object->zpos = (static_cast<double>(positions[2]) / 50.0f) - 1.0f;

        object->uniformBuffers = {};
        object->descriptorSets = {};

        objects.push_back(object);
        i++;
    }
};

void Scene::loadObjects()
{

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    //load objects model vertices and indices into single buffers

    for (auto &object : objects)
    {
        for (auto &vertex : models[object->modelIndex]->vertices)
        {
            vertices.push_back(vertex);
        }
        for (auto &index : models[object->modelIndex]->indices)
        {
            indices.push_back(index);
        }
    }
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    Buffer* stagingBuffer = new Buffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    stagingBuffer->load(vertices);

    vertexBuffer = new Buffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VMA_MEMORY_USAGE_GPU_ONLY);

    stagingBuffer->copy(vertexBuffer);

    bufferSize = sizeof(uint32_t) * indices.size();

    stagingBuffer->resize(bufferSize);
    stagingBuffer->load(indices);

    indexBuffer = new Buffer(state, bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VMA_MEMORY_USAGE_GPU_ONLY);

    stagingBuffer->copy(indexBuffer);

    delete stagingBuffer;

    for (auto &material : materials)
    {
        material->load();
    }
};

void Scene::loadSkybox()
{
    skybox = new Skybox(state, config->skyboxTextures);
};

void Scene::createUniformBuffers()
{
    skybox->createUniformBuffers();

    for (auto &object : objects)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        object->uniformBuffers.resize(state->swapChainImages.size());

        for (size_t i = 0; i < state->swapChainImages.size(); i++)
        {
            object->uniformBuffers[i] = new Buffer(state, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }
    }
};

void Scene::updateUniformBuffer(uint32_t currentImage)
{

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    double moveX = Input::getInstance().getMoveX();
    objects[0]->xpos += moveX;
    if (moveX < 0.0)
    {
        Input::getInstance().setMoveX(moveX + 0.1f);
    }
    else if (moveX > 0.0)
    {
        Input::getInstance().setMoveX(moveX - 0.1f);
    }

    double moveY = Input::getInstance().getMoveY();
    objects[0]->ypos += moveY;
    if (moveY < 0.0)
    {
        Input::getInstance().setMoveY(moveY + 0.1f);
    }
    else if (moveY > 0.0)
    {
        Input::getInstance().setMoveY(moveY - 0.1f);
    }
    for (auto object : objects)
    {
        UniformBufferObject ubo = {};
        ubo.model = glm::rotate(
            glm::translate(glm::mat4(1.0f), glm::vec3(object->xpos, object->ypos, object->zpos)),
            time * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), state->swapChainExtent.width / (float)state->swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        object->uniformBuffers[currentImage]->update(ubo);
    }
}


void Scene::createDescriptorSets()
{
    skybox->createDescriptorSets();

    for (auto object : objects)
    {
        std::vector<VkDescriptorSetLayout> layouts(state->swapChainImages.size(), state->descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = state->descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(state->swapChainImages.size());
        allocInfo.pSetLayouts = layouts.data();

        object->descriptorSets.resize(state->swapChainImages.size());
        if (vkAllocateDescriptorSets(state->device, &allocInfo, object->descriptorSets.data()) != VK_SUCCESS)
        {

            throw std::runtime_error("failed to allocate descript sets");
        }

        for (size_t i = 0; i < state->swapChainImages.size(); i++)
        {

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = object->uniformBuffers[i]->getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = materials[object->materialIndex]->textureImageView;
            imageInfo.sampler = materials[object->materialIndex]->textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = object->descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = object->descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(state->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
};

void Scene::createPipelines()
{
    skybox->createPipeline();

    auto vertShaderCode = readFile("resources/shaders/vert.spv");
    auto fragShaderCode = readFile("resources/shaders/frag.spv");

    VkShaderModule vertShaderModule = state->createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = state->createShaderModule(fragShaderCode);

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

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)state->swapChainExtent.width;
    viewport.height = (float)state->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = state->swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = 0.2f;
    multisampling.rasterizationSamples = state->msaaSamples;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &state->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(state->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("faled to create pipeline layout");
    }

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
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = state->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(state->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(state->device, fragShaderModule, nullptr);
    vkDestroyShaderModule(state->device, vertShaderModule, nullptr);
};