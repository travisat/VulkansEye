#include "Scene.hpp"

Scene::Scene(State *state, Config &config)
{
    this->state = state;

    camera = {};
    camera.setPerspective(config.cameras[0].fieldOfView, static_cast<double>(state->width), static_cast<double>(state->height), 0.1f, 512.0f);
    camera.position = config.cameras[0].position;
    camera.rotation = config.cameras[0].rotation;
    camera.updateView();

    skybox = new Skybox(state, &camera, config.skybox);

    for (auto const &lightConfig : config.lights)
    {
        Light *light = new Light(lightConfig);
        lights.insert({lightConfig.id, light});
    }

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

    for (auto &modelConfig : config.models)
    {
        //if (modelConfig.type == obj)
        //{
            Model *model = new Model(state, modelConfig.id, modelConfig.position, modelConfig.scale,
                                     meshes[modelConfig.meshId], materials[modelConfig.materialId]);
            models.insert({modelConfig.id, model});
        //}
    }
};

Scene::~Scene()
{
    delete vertexBuffer;
    delete indexBuffer;

    vkDestroyPipeline(state->device, pipeline, nullptr);
    vkDestroyPipelineLayout(state->device, pipelineLayout, nullptr);

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

    createUniformBuffers();
    createDescriptorSetLayouts();
    createPipelines();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    //load objects model vertices and indices into single buffers
    for (auto const &[id, model] : models)
    {
        model->vertexOffset = vertexOffset;
        model->indexOffset = indexOffset;
        model->vertexSize = model->mesh->vertexSize;
        model->indexSize = model->mesh->indexSize;

        for (auto &vertex : model->mesh->vertices)
        {
            //converte mesh into size/scale for the model
            //then move to correct initial position
            Vertex out;
            out.position.x = (vertex.position.x * model->scale.x) + model->position.x;
            out.position.y = (vertex.position.y * model->scale.y) + model->position.y;
            out.position.z = (vertex.position.z * model->scale.z) + model->position.z;
            out.UV = vertex.UV;
            out.normal = vertex.normal;

            vertices.push_back(out);
            vertexOffset++;
        }
        for (auto &index : model->mesh->indices)
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
            delete model->uniformLights[i];
        }
    }
}

void Scene::recreate()
{
    camera.updateAspectRatio((float)state->width, (float)state->height);
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
        VkDeviceSize lightSize = sizeof(UniformLightObject);

        model->uniformBuffers.resize(state->swapChainImages.size());
        model->uniformLights.resize(state->swapChainImages.size());

        for (size_t i = 0; i < state->swapChainImages.size(); i++)
        {
            model->uniformBuffers[i] = new Buffer(state, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            model->uniformLights[i] = new Buffer(state, lightSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }
    }
}

void Scene::updateUniformBuffer(uint32_t currentImage)
{
    skybox->updateUniformBuffer(currentImage);

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    for (auto const &[id, model] : models)
    {
        UniformBufferObject ubo = {};
        ubo.model = glm::mat4(1.0f);
        //ubo.model = glm::rotate(ubo.model, time * glm::radians(90.f), model->position);
        ubo.projection = camera.perspective;
        ubo.view = camera.view;
        ubo.cameraPosition = camera.position * -1.0f;

        UniformLightObject ulo = {};
        ulo.lights[0] = lights[0]->light;
        ulo.lights[1] = lights[1]->light;
        ulo.gamma = gamma;
        ulo.exposure = exposure;

        model->uniformBuffers[currentImage]->update(ubo);
        model->uniformLights[currentImage]->update(ulo);
    }
}

void Scene::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uloLayoutBinding = {};
    uloLayoutBinding.binding = 1;
    uloLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uloLayoutBinding.descriptorCount = 1;
    uloLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uloLayoutBinding.pImmutableSamplers = nullptr;

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

    VkDescriptorSetLayoutBinding aoLayoutBinding = {};
    aoLayoutBinding.binding = 5;
    aoLayoutBinding.descriptorCount = 1;
    aoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aoLayoutBinding.pImmutableSamplers = nullptr;
    aoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 6> bindings = {uboLayoutBinding, uloLayoutBinding, diffuseLayoutBinding, normalLayoutBinding, roughnessLayoutBinding, aoLayoutBinding};

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

            VkDescriptorBufferInfo lightInfo = {};
            lightInfo.buffer = model->uniformLights[i]->buffer;
            lightInfo.offset = 0;
            lightInfo.range = sizeof(UniformLightObject);

            VkDescriptorImageInfo diffuseInfo = {};
            diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            diffuseInfo.imageView = model->material->diffuse->imageView;
            diffuseInfo.sampler = model->material->diffuseSampler;

            VkDescriptorImageInfo normalInfo = {};
            normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normalInfo.imageView = model->material->normal->imageView;
            normalInfo.sampler = model->material->normalSampler;

            VkDescriptorImageInfo roughnessInfo = {};
            roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            roughnessInfo.imageView = model->material->roughness->imageView;
            roughnessInfo.sampler = model->material->roughnessSampler;

            VkDescriptorImageInfo aoInfo = {};
            aoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            aoInfo.imageView = model->material->ao->imageView;
            aoInfo.sampler = model->material->aoSampler;

            std::array<VkWriteDescriptorSet, 6> descriptorWrites = {};
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
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pBufferInfo = &lightInfo;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = model->descriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pImageInfo = &diffuseInfo;

            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[3].dstSet = model->descriptorSets[i];
            descriptorWrites[3].dstBinding = 3;
            descriptorWrites[3].dstArrayElement = 0;
            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[3].descriptorCount = 1;
            descriptorWrites[3].pImageInfo = &normalInfo;

            descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[4].dstSet = model->descriptorSets[i];
            descriptorWrites[4].dstBinding = 4;
            descriptorWrites[4].dstArrayElement = 0;
            descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[4].descriptorCount = 1;
            descriptorWrites[4].pImageInfo = &roughnessInfo;

            descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[5].dstSet = model->descriptorSets[i];
            descriptorWrites[5].dstBinding = 5;
            descriptorWrites[5].dstArrayElement = 0;
            descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[5].descriptorCount = 1;
            descriptorWrites[5].pImageInfo = &aoInfo;

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
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
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