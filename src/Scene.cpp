#include "Scene.h"

Scene::Scene(State *state, Config &config)
{
    this->state = state;

    camera = {};
    camera.setPerspective(60.0f, static_cast<double>(state->windowWidth), static_cast<double>(state->windowHeight), 0.1f, 512.0f);
    camera.position = glm::vec3(0.0f, 0.0f, -5.0f);
    camera.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.updateView();

    skybox = new Skybox(state, &camera, config.skybox);

    for (auto const &meshConfig : config.meshes)
    {
        Mesh *mesh = new Mesh(meshConfig);
        meshes.insert({meshConfig.id, mesh});
    }

    for (auto const &materialConfig : config.materials)
    {
        Material *material = new Material(state, materialConfig);
        materials.insert({materialConfig.id, material});
    }

    for (auto &modelConfig : config.modelConfigs)
    {
        //convert from 0-100 to -1.0 to 1.0
        double xpos = (static_cast<double>(modelConfig.xpos) / 50.0f) - 1.0f;
        double ypos = (static_cast<double>(modelConfig.ypos) / 50.0f) - 1.0f;
        double zpos = (static_cast<double>(modelConfig.zpos) / 50.0f) - 1.0f;

        Model *model = new Model(state, meshes[modelConfig.meshId],
                                 materials[modelConfig.materialId], glm::vec3(xpos, ypos, zpos));

        models.insert({modelConfig.id, model});
    }
};

Scene::~Scene()
{
    delete vertexBuffer;
    delete indexBuffer;

    vkDestroyDescriptorSetLayout(state->device, descriptorSetLayout, nullptr);

    for (auto const &[id, model] : models)
    {
        delete model;
    };

    for (auto const &[id, mesh] : meshes)
    {
        delete mesh;
    };
    for (auto const &[id, material] : materials)
    {
        delete material;
    };
    delete skybox;
}

void Scene::create()
{
    skybox->create();

    createDescriptorSetLayouts();
    createPipelines();
    createUniformBuffers();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    //load objects model vertices and indices into single buffers
    for (auto const &[id, mesh] : meshes)
    {
        mesh->vertexOffset = vertexOffset;
        mesh->indexOffset = indexOffset;

        for (auto &vertex : mesh->vertices)
        {
            vertices.push_back(vertex);
            vertexOffset++;
        }
        for (auto &index : mesh->indices)
        {
            indices.push_back(index);
            indexOffset++;
        }
    }

    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    Buffer *stagingBuffer = new Buffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
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

    for (auto const &[id, material] : materials)
    {
        material->load();
    }

    createDescriptorSets();
}

void Scene::cleanup()
{
    skybox->cleanup();
    vkDestroyPipeline(state->device, pipeline, nullptr);
    vkDestroyPipelineLayout(state->device, pipelineLayout, nullptr);
    for (uint32_t i = 0; i < state->swapChainImages.size(); i++)
    {
        for (auto const &[id, model] : models)
        {
            delete model->uniformBuffers[i];
        }
    }
}

void Scene::recreate()
{
    camera.updateAspectRatio((float)state->windowWidth, (float)state->windowHeight);
    skybox->recreate();
    createPipelines();
    createUniformBuffers();
    createDescriptorSets();
}

void Scene::createUniformBuffers()
{

    for (auto const &[_, model] : models)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        model->uniformBuffers.resize(state->swapChainImages.size());

        for (size_t i = 0; i < state->swapChainImages.size(); i++)
        {
            model->uniformBuffers[i] = new Buffer(state, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }
    }
}

void Scene::updateUniformBuffer(uint32_t currentImage)
{
    skybox->updateUniformBuffer(currentImage);

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    if (Input::getInstance().getMouseMode())
    {
        camera.update(Input::getKeys(), glm::vec2(Input::getInstance().getMouseX(), Input::getInstance().getMouseY()), time);
    }
    else
    {
        camera.changeMouseMode(false);
    }

    for (auto const &[id, model] : models)
    {
        UniformBufferObject ubo = {};
        ubo.model = glm::translate(glm::mat4(1.0f), model->position);
        ubo.model = glm::rotate(ubo.model, time * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.projection = camera.perspective;
        ubo.view = camera.view;
        ubo.cameraPosition = camera.position * -1.0f;

        model->uniformBuffers[currentImage]->update(ubo);
    }
}

void Scene::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(state->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("faled to create descriptor set layout");
    }
}

void Scene::createDescriptorSets()
{
    for (auto const &[id, model] : models)
    {
        std::vector<VkDescriptorSetLayout> layouts(state->swapChainImages.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = state->descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(state->swapChainImages.size());
        allocInfo.pSetLayouts = layouts.data();

        model->descriptorSets.resize(state->swapChainImages.size());
        if (vkAllocateDescriptorSets(state->device, &allocInfo, model->descriptorSets.data()) != VK_SUCCESS)
        {

            throw std::runtime_error("failed to allocate descript sets");
        }

        for (size_t i = 0; i < state->swapChainImages.size(); i++)
        {

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = model->uniformBuffers[i]->buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = model->material->diffuse->imageView;
            imageInfo.sampler = model->material->diffuseSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = model->descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = model->descriptorSets[i];
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
    auto vertShaderCode = readFile("resources/shaders/scene.vert.spv");
    auto fragShaderCode = readFile("resources/shaders/scene.frag.spv");

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
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;


    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
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
    pipelineInfo.pDynamicState = &dynamicState;
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