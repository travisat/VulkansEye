#include "Scene.hpp"
#include "State.hpp"
namespace tat
{

//can't be constructor cause models require pointer to scene which wouldn't exist yet
void Scene::load() {
    createBrdf();
    createShadow();
    loadBackdrop();
    loadModels();

    createColorPool(); // needs stage/lights/actors to know number of descriptors
    createColorLayouts();
    createColorSets();     // needs descriptorsetLayout and descriptorpool
    createColorPipeline(); // needs descriptorsetlayout
    createShadowPool();
    createShadowLayouts();
    createShadowSets();
    createShadowPipeline();
    spdlog::info("Loaded Scene");
}

Scene::~Scene()
{
    auto &state = State::instance();
    if (shadowLayout)
    {
        state.vulkan->device.destroyDescriptorSetLayout(shadowLayout);
    }
    if (colorLayout)
    {
        state.vulkan->device.destroyDescriptorSetLayout(colorLayout);
    }
    if (colorPool)
    {
        state.vulkan->device.destroyDescriptorPool(colorPool);
    }
    if (shadowPool)
    {
        state.vulkan->device.destroyDescriptorPool(shadowPool);
    }
}

void Scene::createBrdf()
{
    auto &settings = State::instance().at("settings");
    brdf = std::make_shared<Image>();
    brdf->imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    brdf->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    brdf->load(settings.at("brdfPath"));

    brdf->samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    brdf->samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    brdf->samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    brdf->samplerInfo.maxAnisotropy = 1.0F;
    brdf->samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    brdf->createSampler();
    spdlog::info("Created BRDF");
}

void Scene::createShadow()
{
    auto &settings = State::instance().at("settings");
    shadowSize = settings.at("shadowSize");
    shadow = std::make_shared<Image>();
    shadow->imageInfo.format = vk::Format::eR32G32Sfloat;
    shadow->imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
    shadow->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadow->resize(static_cast<int>(shadowSize), static_cast<int>(shadowSize));
    shadow->transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    shadow->samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    shadow->samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    shadow->samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    shadow->samplerInfo.maxAnisotropy = 1.F;
    shadow->samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    shadow->createSampler();
    spdlog::info("Created Shadow");
}

void Scene::loadBackdrop()
{
    auto &state = State::instance();
    backdrop = state.backdrops->get(state.at("scene").at("backdrop"));
}

void Scene::loadModels()
{
    auto &state = State::instance();
    auto &scene = state.at("scene");
    for (auto &model : scene.at("models"))
    {
        models.push_back(state.models->get(model.get<std::string>()));
        spdlog::info("Loaded Model {}", model.get<std::string>());
    }
    spdlog::info("Created Models");
}

void Scene::cleanup()
{
    auto &state = State::instance();
    backdrop->cleanup();
    colorPipeline.cleanup();
    state.vulkan->device.destroyDescriptorPool(colorPool);
}

void Scene::recreate()
{
    auto &state = State::instance();
    auto &window = state.at("settings").at("window");
    state.camera->updateProjection(window.at(0), window.at(1));

    backdrop->recreate();
    createColorPool();
    createColorSets();
    createColorPipeline();
}

void Scene::drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    backdrop->draw(commandBuffer, currentImage);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, colorPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        auto mesh = model->getMesh();
        commandBuffer.bindVertexBuffers(0, 1, &mesh->buffers.vertex.buffer, offsets.data());
        commandBuffer.bindIndexBuffer(mesh->buffers.index.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, colorPipeline.pipelineLayout, 0, 1,
                                         &model->colorSets[currentImage], 0, nullptr);
        commandBuffer.drawIndexed(mesh->data.indices.size(), 1, 0, 0, 0);
    }
}

void Scene::drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        auto mesh = model->getMesh();
        commandBuffer.bindVertexBuffers(0, 1, &mesh->buffers.vertex.buffer, offsets.data());
        commandBuffer.bindIndexBuffer(mesh->buffers.index.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowPipeline.pipelineLayout, 0, 1,
                                         &model->shadowSets[currentImage], 0, nullptr);
        commandBuffer.drawIndexed(mesh->data.indices.size(), 1, 0, 0, 0);
    }
}

void Scene::update(uint32_t currentImage, float deltaTime)
{
    auto &state = State::instance();
    backdrop->update(currentImage);

    fragBuffer.position = glm::vec4(backdrop->light, 1.F);

    glm::mat4 depthProjectionMatrix = glm::ortho(-30.F, 30.F, -30.F, 30.F, state.camera->zNear,
                                                 state.camera->zFar);
    glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(fragBuffer.position), glm::vec3(0.F), glm::vec3(0, 1, 0));

    fragBuffer.radianceMipLevels = backdrop->radianceMap->imageInfo.mipLevels;
    fragBuffer.shadowSize = shadowSize;

    for (auto &model : models)
    {
        model->update(deltaTime);
        // create mvp for player space
        vertBuffer.model = model->model();
        vertBuffer.view = state.camera->view();
        vertBuffer.projection = state.camera->projection();
        vertBuffer.normalMatrix =
            glm::transpose(glm::inverse(state.camera->projection() * state.camera->view() * model->model()));
        vertBuffer.camPos = glm::vec4(-state.camera->position(), 1.F);

        // create mvp for lightspace
        shadBuffer.model = model->model();
        shadBuffer.view = depthViewMatrix;
        shadBuffer.projection = depthProjectionMatrix;
        vertBuffer.lightMVP = depthProjectionMatrix * depthViewMatrix * model->model();
        model->vertBuffers[currentImage].update(&vertBuffer, sizeof(vertBuffer));
        model->fragBuffers[currentImage].update(&fragBuffer, sizeof(fragBuffer));
        model->shadBuffers[currentImage].update(&shadBuffer, sizeof(shadBuffer));
    }
}

void Scene::createColorPool()
{
    auto &state = State::instance();
    auto numSwapChainImages = static_cast<uint32_t>(state.vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    // number of models * uniform buffers * swapchainimages
    poolSizes[0].descriptorCount = models.size() * (2) * numSwapChainImages;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    // number of models * imagesamplers * swapchainimages
    poolSizes[1].descriptorCount = models.size() * 9 * numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // number of models * swapchainimages
    poolInfo.maxSets = models.size() * numSwapChainImages;

    colorPool = state.vulkan->device.createDescriptorPool(poolInfo);
}

void Scene::createColorLayouts()
{
    auto &state = State::instance();
    std::array<vk::DescriptorSetLayoutBinding, 11> bindings{};

    // UniformBuffer
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;

    // uLight
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // shadow
    bindings[2].binding = 2;
    bindings[2].descriptorCount = 1;
    bindings[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[2].pImmutableSamplers = nullptr;
    bindings[2].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // diffuse
    bindings[3].binding = 3;
    bindings[3].descriptorCount = 1;
    bindings[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[3].pImmutableSamplers = nullptr;
    bindings[3].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // normal
    bindings[4].binding = 4;
    bindings[4].descriptorCount = 1;
    bindings[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[4].pImmutableSamplers = nullptr;
    bindings[4].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // roughness
    bindings[5].binding = 5;
    bindings[5].descriptorCount = 1;
    bindings[5].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[5].pImmutableSamplers = nullptr;
    bindings[5].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // metallic
    bindings[6].binding = 6;
    bindings[6].descriptorCount = 1;
    bindings[6].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[6].pImmutableSamplers = nullptr;
    bindings[6].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // ao
    bindings[7].binding = 7;
    bindings[7].descriptorCount = 1;
    bindings[7].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[7].pImmutableSamplers = nullptr;
    bindings[7].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // irradiance
    bindings[8].descriptorCount = 1;
    bindings[8].binding = 8;
    bindings[8].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[8].pImmutableSamplers = nullptr;
    bindings[8].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // radiance
    bindings[9].descriptorCount = 1;
    bindings[9].binding = 9;
    bindings[9].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[9].pImmutableSamplers = nullptr;
    bindings[9].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // brdf pregenned texture
    bindings[10].descriptorCount = 1;
    bindings[10].binding = 10;
    bindings[10].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[10].pImmutableSamplers = nullptr;
    bindings[10].stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    colorLayout = state.vulkan->device.createDescriptorSetLayout(layoutInfo);
}

void Scene::createColorSets()
{

    for (auto &model : models)
    {
        model->createColorSets(colorPool, colorLayout);
    }
}

void Scene::createColorPipeline()
{
    auto &state = State::instance();
    colorPipeline.descriptorSetLayout = colorLayout;
    colorPipeline.loadDefaults(state.vulkan->colorPass);

    auto vertPath = "assets/shaders/scene.vert.spv";
    auto fragPath = "assets/shaders/scene.frag.spv";

    colorPipeline.vertShaderStageInfo.module = state.vulkan->createShaderModule(vertPath);
    colorPipeline.fragShaderStageInfo.module = state.vulkan->createShaderModule(fragPath);

    colorPipeline.shaderStages = {colorPipeline.vertShaderStageInfo, colorPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    colorPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    colorPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    colorPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    colorPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    colorPipeline.create();
}

void Scene::createShadowPool()
{
    auto &state = State::instance();
    auto numSwapChainImages = static_cast<uint32_t>(state.vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    // number of models * uniformBuffers * swapchainimages
    poolSizes[0].descriptorCount = models.size() * 1 * numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // number of models * swapchainimages
    poolInfo.maxSets = models.size() * numSwapChainImages;

    shadowPool = state.vulkan->device.createDescriptorPool(poolInfo);
}

void Scene::createShadowLayouts()
{
    auto &state = State::instance();
    vk::DescriptorSetLayoutBinding shadowLayoutBinding = {};
    shadowLayoutBinding.binding = 0;
    shadowLayoutBinding.descriptorCount = 1;
    shadowLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    shadowLayoutBinding.pImmutableSamplers = nullptr;
    shadowLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    std::array<vk::DescriptorSetLayoutBinding, 1> layouts = {shadowLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.bindingCount = static_cast<int32_t>(layouts.size());
    layoutInfo.pBindings = layouts.data();

    shadowLayout = state.vulkan->device.createDescriptorSetLayout(layoutInfo);
}

void Scene::createShadowSets()
{
    for (auto &model : models)
    {
        model->createShadowSets(shadowPool, shadowLayout);
    }
}

void Scene::createShadowPipeline()
{
    auto &state = State::instance();
    shadowPipeline.descriptorSetLayout = shadowLayout;
    shadowPipeline.loadDefaults(state.vulkan->shadowPass);

    auto vertPath = "assets/shaders/shadow.vert.spv";
    auto fragPath = "assets/shaders/shadow.frag.spv";

    shadowPipeline.vertShaderStageInfo.module = state.vulkan->createShaderModule(vertPath);
    shadowPipeline.fragShaderStageInfo.module = state.vulkan->createShaderModule(fragPath);

    shadowPipeline.shaderStages = {shadowPipeline.vertShaderStageInfo, shadowPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    shadowPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    shadowPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    shadowPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    shadowPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    shadowPipeline.multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    shadowPipeline.rasterizer.cullMode = vk::CullModeFlagBits::eFront;

    shadowPipeline.create();
}

} // namespace tat