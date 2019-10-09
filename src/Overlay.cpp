#include "Overlay.hpp"
#include "helpers.h"

Overlay::~Overlay()
{
    ImGui::DestroyContext();
    vkDestroySampler(vulkan->device, sampler, nullptr);
    vkDestroyDescriptorSetLayout(vulkan->device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(vulkan->device, pipeline, nullptr);
    vkDestroyPipelineLayout(vulkan->device, pipelineLayout, nullptr);
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Overlay::create()
{
    ImGui::CreateContext();
    // Color scheme
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    // Dimensions
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)vulkan->width, (float)vulkan->height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    
    createFont();

    createDescriptorPool();
    createDescriptorLayouts();
    createDescriptorSets();
    createPipelineLayout();
    createPipeline();
    newFrame(true);
    createBuffers();
}

void Overlay::recreate()
{
    createPipelineLayout();
    createPipeline(); 
    createDescriptorPool();
    createDescriptorSets();
}

void Overlay::cleanup()
{
    vkDestroyPipeline(vulkan->device, pipeline, nullptr);
    vkDestroyPipelineLayout(vulkan->device, pipelineLayout, nullptr);
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Overlay::createBuffers()
{
    vertexBuffer.flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vertexBuffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    vertexBuffer.name = "overlay vertex buffer";
    indexBuffer.flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    indexBuffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    indexBuffer.name = "overlay index buffer";
}

void Overlay::createFont()
{
    ImGuiIO &io = ImGui::GetIO();
    unsigned char *fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

    fontImage.vulkan = vulkan;
    fontImage.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    fontImage.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    fontImage.width = texWidth;
    fontImage.height = texHeight;
    fontImage.name = "overlay fontimage";
    fontImage.allocate();

    fontImage.createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    // Staging buffers for font data upload
    Buffer stagingBuffer;
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    stagingBuffer.name = "overlay font image staging buffer";

    stagingBuffer.load(gsl::make_span(fontData, fontData + static_cast<uint32_t>(uploadSize)));

    fontImage.transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy buffer data to font image
    VkCommandBuffer copyCmd = vulkan->beginSingleTimeCommands();

    // Copy
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = texWidth;
    bufferCopyRegion.imageExtent.height = texHeight;
    bufferCopyRegion.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        copyCmd,
        stagingBuffer.buffer,
        fontImage.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &bufferCopyRegion);

    vulkan->endSingleTimeCommands(copyCmd);

    fontImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Font texture Sampler
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vkCreateSampler(vulkan->device, &samplerInfo, nullptr, &sampler);
}

void Overlay::createDescriptorPool()
{
    uint32_t numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<VkDescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // set max set size to which set is larger
    poolInfo.maxSets = numSwapChainImages;

    CheckResult(vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr, &descriptorPool));
    //Trace("Created overlay descriptor pool at ", Timer::systemTime());
}

void Overlay::createDescriptorLayouts()
{
    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr, &descriptorSetLayout));
}

void Overlay::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(vulkan->swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(vulkan->swapChainImages.size());
    CheckResult(vkAllocateDescriptorSets(vulkan->device, &allocInfo, descriptorSets.data()));

    for (size_t i = 0; i < vulkan->swapChainImages.size(); i++)
    {
        VkDescriptorImageInfo samplerInfo = {};
        samplerInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        samplerInfo.imageView = fontImage.imageView;
        samplerInfo.sampler = sampler;

        std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &samplerInfo;

        vkUpdateDescriptorSets(vulkan->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Overlay::createPipelineLayout()
{
    // Pipeline layout
    // Push constants for UI rendering parameters
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.size = sizeof(PushConstBlock);
    pushConstantRange.offset = 0;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

    CheckResult(vkCreatePipelineLayout(vulkan->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void Overlay::createPipeline()
{
    // Vertex bindings an attributes based on ImGui vertex definition
    auto vertShaderCode = tat::readFile("resources/shaders/ui.vert.spv");
    auto fragShaderCode = tat::readFile("resources/shaders/ui.frag.spv");

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

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ImDrawVert);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R8G8B8_UNORM;
    attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &bindingDescription;
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Setup graphics pipeline for UI rendering
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    // Enable blending
    VkPipelineColorBlendAttachmentState blendAttachmentState = {};
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &blendAttachmentState;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_FALSE;
    depthStencilState.depthWriteEnable = VK_FALSE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vulkan->msaaSamples;

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.renderPass = vulkan->renderPass;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampling;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.layout = pipelineLayout;

    CheckResult(vkCreateGraphicsPipelines(vulkan->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));

    vkDestroyShaderModule(vulkan->device, fragShaderModule, nullptr);
    vkDestroyShaderModule(vulkan->device, vertShaderModule, nullptr);
}

void Overlay::newFrame(bool updateFrameGraph)
{
    ImGui::NewFrame();

    static float f = 0.0f;
    ImGui::TextUnformatted(vulkan->name.c_str());

    // Update frame time display
    if (updateFrameGraph)
    {
        std::rotate(uiSettings.frameTimes.begin(), uiSettings.frameTimes.begin() + 1, uiSettings.frameTimes.end());
        float frameTime = (float)(Timer::getCount() * 1000.0f) - lastFrametime;
        lastFrametime = frameTime;
        uiSettings.frameTimes.back() = frameTime;
        if (frameTime < uiSettings.frameTimeMin)
        {
            uiSettings.frameTimeMin = frameTime;
        }
        if (frameTime > uiSettings.frameTimeMax)
        {
            uiSettings.frameTimeMax = frameTime;
        }
    }

    ImGui::PlotLines("Frame Times", &uiSettings.frameTimes[0], 50, 0, "", uiSettings.frameTimeMin, uiSettings.frameTimeMax, ImVec2(0, 80));

    ImGui::Text("Camera");
    ImGui::InputFloat3("position", &player->position.x, 2);
    ImGui::InputFloat3("rotation", &player->rotation.x, 2);
    ImGui::InputFloat3("velocity", &player->velocity.x, 2);
    ImGui::InputFloat3("force", &player->force.x, 2);

    /*
        ImGui::SetNextWindowSize(ImVec2(550, 380), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("Example settings");
        ImGui::Checkbox("Render models", &uiSettings.displayModels);
        ImGui::Checkbox("Display logos", &uiSettings.displayLogos);
        ImGui::Checkbox("Display background", &uiSettings.displayBackground);
        ImGui::Checkbox("Animate light", &uiSettings.animateLight);
        ImGui::SliderFloat("Light speed", &uiSettings.lightSpeed, 0.1f, 1.0f);
        ImGui::End();
        ImGui::SetNextWindowSize(ImVec2(400, 380), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
*/
    //ImGui::ShowDemoWindow();

    // Render to generate draw buffers
    ImGui::Render();
}

void Overlay::updateBuffers()
{
    ImDrawData *imDrawData = ImGui::GetDrawData();

    if (imDrawData)
    {
        // Note: Alignment is done inside buffer creation
        VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        // Update buffers only if vertex or index count has been changed compared to current buffer size

        update = (vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount) || (indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount != imDrawData->TotalIdxCount);

        if (update)
        {
            vkDeviceWaitIdle(vulkan->device);

            vertexBuffer.vulkan = vulkan;
            vertexBuffer.resize(vertexBufferSize);
            vertexCount = imDrawData->TotalVtxCount;

            indexBuffer.vulkan = vulkan;
            indexBuffer.resize(indexBufferSize);
            indexCount = imDrawData->TotalIdxCount;
        }
        // Upload data
        ImDrawVert *vtxDst = (ImDrawVert *)vertexBuffer.mapped;
        ImDrawIdx *idxDst = (ImDrawIdx *)indexBuffer.mapped;

        for (int n = 0; n < imDrawData->CmdListsCount; n++)
        {
            const ImDrawList *cmd_list = imDrawData->CmdLists[n];
            memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += cmd_list->VtxBuffer.Size;
            idxDst += cmd_list->IdxBuffer.Size;
        }

        // Flush to make writes visible to GPU
        vertexBuffer.flush();
        indexBuffer.flush();
    }
}

void Overlay::draw(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    if (!vertexBuffer.buffer || !indexBuffer.buffer)
    {
        newFrame(true);
        updateBuffers();
    }
    ImGuiIO &io = ImGui::GetIO();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentImage], 0, nullptr);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport = {};
    viewport.width = ImGui::GetIO().DisplaySize.x;
    viewport.height = ImGui::GetIO().DisplaySize.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // UI scale and translate via push constants
    pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    pushConstBlock.translate = glm::vec2(-1.0f);
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

    // Render commands
    ImDrawData *imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
    {
        const ImDrawList *cmd_list = imDrawData->CmdLists[i];
        for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
        {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[j];
            VkRect2D scissorRect;
            scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
            scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
            scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
            scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
            vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}