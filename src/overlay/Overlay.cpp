#include "overlay/Overlay.hpp"
#include "Input.hpp"
#include "State.hpp"
#include "Timer.hpp"
#include "engine/Debug.hpp"

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <filesystem>
#include <memory>

#include <spdlog/spdlog.h>

namespace tat
{

static auto ImGui_ImplGlfw_GetClipboardText(void *user_data) -> const char *
{
    return glfwGetClipboardString(reinterpret_cast<GLFWwindow *>(user_data));
}

static void ImGui_ImplGlfw_SetClipboardText(void *user_data, const char *text)
{
    glfwSetClipboardString(reinterpret_cast<GLFWwindow *>(user_data), text);
}

void Overlay::create()
{
    ImGui::CreateContext();
    // Color scheme
    auto &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.F, 0.3F, 0.3F, 0.6F);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.F, 0.4F, 0.4F, 0.8F);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.4F, 0.4F, 0.4F, 0.4F);
    style.Colors[ImGuiCol_Header] = ImVec4(0.4F, 0.4F, 0.4F, 0.4F);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.F, 1.F, 1.F, 1.F);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13F, 0.1F, 0.12F, 0.95F);
    style.WindowRounding = 0.F;
    style.WindowPadding = ImVec2(2.F, 2.F);
    style.WindowBorderSize = 1.F;
    style.Alpha = 0.5F;
    style.FrameRounding = 0.F;
    style.FrameBorderSize = 0.F;

    // Dimensions
    window = &State::instance().window;
    io = &ImGui::GetIO();
    io->DisplaySize = ImVec2(window->width, window->height);
    io->DisplayFramebufferScale = ImVec2(1.F, 1.F);

    // Input
    io->BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
    io->BackendPlatformName = "imgui-vulkanseye";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io->KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io->KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io->KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io->KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io->KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io->KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io->KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io->KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io->KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io->KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io->KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io->KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io->KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io->KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io->KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io->KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io->KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io->KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io->KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io->KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io->KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io->KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io->SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    io->GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
    io->ClipboardUserData = window->window;
#if defined(_WIN32)
    io->ImeWindowHandle = (void *)glfwGetWin32Window(window->window);
#endif

    g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    createFont();

    createDescriptorPool();
    createDescriptorLayouts();
    createDescriptorSets();
    createPipeline();
    createBuffers();

    info.create();
    editor.create();
    paused.create();

    update(0.F);

    if constexpr (Debug::enable)
    {
        spdlog::info("Created Overlay");
    }
}

void Overlay::destroy()
{
    ImGui::DestroyContext();

    auto &device = State::instance().engine.device;

    fontImage.destroy();
    vertexBuffer.destroy();
    indexBuffer.destroy();
    device.destroy(descriptorSetLayout);
    device.destroy(descriptorPool);
    pipeline.destroy();

    if constexpr (Debug::enable)
    {
        spdlog::info("Destroyed Overlay");
    }
}

void Overlay::recreate()
{
    io->DisplaySize = ImVec2(window->width, window->height);

    createDescriptorPool();
    createDescriptorSets();
    createPipeline();
}

void Overlay::cleanup()
{
    auto &device = State::instance().engine.device;
    device.destroy(descriptorPool);
    pipeline.destroy();
}

void Overlay::createBuffers()
{
    vertexBuffer.flags = vk::BufferUsageFlagBits::eVertexBuffer;
    vertexBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vertexBuffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    indexBuffer.flags = vk::BufferUsageFlagBits::eIndexBuffer;
    indexBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    indexBuffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if constexpr (Debug::enable)
    {
        vertexBuffer.name = "Overlay Vert";
        indexBuffer.name = "Overlay index";
    }
}

void Overlay::createFont()
{
    auto &engine = State::instance().engine;
    auto &io = ImGui::GetIO();
    unsigned char *fontData;
    int texWidth;
    int texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    auto uploadSize = texWidth * texHeight * 4 * sizeof(char);

    fontImage.imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    fontImage.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    fontImage.resize(texWidth, texHeight);
    fontImage.transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    // Staging buffers for font data upload
    Buffer stagingBuffer{};
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    if constexpr (Debug::enable)
    {
        stagingBuffer.name = "Overlay Staging";
    }
    stagingBuffer.update(fontData, uploadSize);

    // Copy buffer data to font image
    auto copyCmd = engine.beginSingleTimeCommands();

    // Copy
    vk::BufferImageCopy bufferCopyRegion{};
    bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = texWidth;
    bufferCopyRegion.imageExtent.height = texHeight;
    bufferCopyRegion.imageExtent.depth = 1;

    copyCmd.copyBufferToImage(stagingBuffer.buffer, fontImage.image, vk::ImageLayout::eTransferDstOptimal, 1,
                              &bufferCopyRegion);

    engine.endSingleTimeCommands(copyCmd);

    fontImage.transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    // Font texture Sampler
    fontImage.samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    fontImage.samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    fontImage.samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    fontImage.samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    fontImage.createSampler();
}

void Overlay::createDescriptorPool()
{
    auto &engine = State::instance().engine;

    std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[0].descriptorCount = engine.swapChain.count;

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = engine.swapChain.count;

    descriptorPool = engine.device.create(poolInfo);

    if constexpr (Debug::enable)
    { // only do this if debug is enabled
        Debug::setName(engine.device.device, descriptorPool, "Overlay Pool");
    }
}

void Overlay::createDescriptorLayouts()
{
    auto &engine = State::instance().engine;
    vk::DescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {samplerBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = engine.device.create(layoutInfo);

    if constexpr (Debug::enable)
    { // only do this if debug is enabled
        Debug::setName(engine.device.device, descriptorSetLayout, "Overlay Layout");
    }
}

void Overlay::createDescriptorSets()
{
    auto &engine = State::instance().engine;
    std::vector<vk::DescriptorSetLayout> layouts(engine.swapChain.count, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = engine.swapChain.count;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets = engine.device.create(allocInfo);

    if constexpr (Debug::enable)
    { // only do this if validation is enabled
        for (auto &descriptorSet : descriptorSets)
        {
            Debug::setName(engine.device.device, descriptorSet, "Overlay Descriptor Set");
        }
    }

    for (size_t i = 0; i < engine.swapChain.count; i++)
    {
        vk::DescriptorImageInfo samplerInfo{};
        samplerInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        samplerInfo.imageView = fontImage.imageView;
        samplerInfo.sampler = fontImage.sampler;

        std::vector<vk::WriteDescriptorSet> descriptorWrites(1);
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &samplerInfo;

        engine.device.update(descriptorWrites);
    }
}

void Overlay::createPipeline()
{
    auto &engine = State::instance().engine;
    pipeline.descriptorSetLayout = &descriptorSetLayout;

    auto vertPath = "assets/shaders/ui.vert.spv";
    auto fragPath = "assets/shaders/ui.frag.spv";
    pipeline.vertShader = engine.createShaderModule(vertPath);
    pipeline.fragShader = engine.createShaderModule(fragPath);

    pipeline.loadDefaults(engine.colorPass.renderPass);

    // Push constants for UI rendering parameters
    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
    pushConstantRange.size = sizeof(PushConstBlock);
    pushConstantRange.offset = 0;

    pipeline.pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipeline.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    pipeline.shaderStages = {pipeline.vertShaderStageInfo, pipeline.fragShaderStageInfo};

    vk::VertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ImDrawVert);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;

    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR8G8B8Unorm;
    attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

    pipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    pipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    pipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipeline.rasterizer.cullMode = vk::CullModeFlagBits::eNone;

    // Enable blending
    pipeline.colorBlendAttachment.blendEnable = VK_TRUE;
    pipeline.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    pipeline.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    pipeline.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    pipeline.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    pipeline.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    pipeline.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

    pipeline.depthStencil.depthTestEnable = VK_FALSE;
    pipeline.depthStencil.depthWriteEnable = VK_FALSE;

    pipeline.create();

    if constexpr (Debug::enable)
    { // only do this if validation is enabled
        Debug::setName(engine.device.device, pipeline.vertShader, "Overlay Vert Shader");
        Debug::setName(engine.device.device, pipeline.fragShader, "Overlay Frag Shader");
        Debug::setName(engine.device.device, pipeline.pipeline, "Overlay Pipeline");
        Debug::setName(engine.device.device, pipeline.pipelineLayout, "Overlay PipelineLayout");
    }
}

void Overlay::update(float deltaTime)
{
    io->DisplaySize = ImVec2(window->width, window->height);
    if (deltaTime > 0.F)
    {
        io->DeltaTime = deltaTime;
    }

    ImGui::NewFrame();

    if (settings.showEditor)
    {
        editor.show();
    }
    if (settings.showInfo)
    {
        info.show(deltaTime);
    }
    if (settings.showPaused)
    {
        paused.show();
    }

    ImGui::Render();

    auto *imDrawData = ImGui::GetDrawData();
    if (imDrawData != nullptr)
    {
        // recreate buffers only if vertex or index size has been changed
        auto vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        auto indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
        auto &engine = State::instance().engine;
        if (vertexBuffer.getSize() < vertexBufferSize)
        {
            engine.device.wait();
            vertexBuffer.create(vertexBufferSize);
            engine.updateCommandBuffer = true;
        }
        if (indexBuffer.getSize() < indexBufferSize)
        {
            engine.device.wait();
            indexBuffer.create(indexBufferSize);
            engine.updateCommandBuffer = true;
        }

        // Upload data
        auto *vtxDst = reinterpret_cast<ImDrawVert *>(vertexBuffer.mapped);
        auto *idxDst = reinterpret_cast<ImDrawVert *>(indexBuffer.mapped);
        for (int n = 0; n < imDrawData->CmdListsCount; n++)
        {
            const auto *cmd_list = imDrawData->CmdLists[n];
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

void Overlay::draw(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    if ((!vertexBuffer.buffer) || (!indexBuffer.buffer))
    {
        update(0.F);
    }
    auto &io = ImGui::GetIO();
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0, 1,
                                     &descriptorSets[currentImage], 0, nullptr);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);

    // UI scale and translate via push constants
    pushConstBlock.scale = glm::vec2(2.F / io.DisplaySize.x, 2.F / io.DisplaySize.y);
    pushConstBlock.translate = glm::vec2(-1.F);
    commandBuffer.pushConstants(pipeline.pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstBlock),
                                &pushConstBlock);

    // Render commands
    auto *imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;

    std::array<vk::DeviceSize, 1> offsets{};
    commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer, offsets.data());
    commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint16);

    for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
    {
        const auto *cmd_list = imDrawData->CmdLists[i];
        for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
        {
            const auto *pcmd = &cmd_list->CmdBuffer[j];
            vk::Rect2D scissorRect{};
            scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
            scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
            scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
            scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
            commandBuffer.setScissor(0, 1, &scissorRect);
            commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

} // namespace tat