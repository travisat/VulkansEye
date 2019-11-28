#include "Model.hpp"
#include "State.hpp"
#include "engine/Debug.hpp"

#include <exception>
#include <memory>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace tat
{

void Model::load()
{
    auto &state = State::instance();
    auto &model = state.at("models").at(name);

    // get material/mesh from their collections
    material = state.materials.get(model.at("material"));
    mesh = state.meshes.get(model.at("mesh"));
    m_size = mesh->size;
    m_mass = model.at("mass");

    // move/rotate/scale
    translate(glm::vec3(model.at("position").at(0), model.at("position").at(1), model.at("position").at(2)));
    rotate(glm::vec3(model.at("rotation").at(0), model.at("rotation").at(1), model.at("rotation").at(2)));
    scale(glm::vec3(model.at("scale").at(0), model.at("scale").at(1), model.at("scale").at(2)));
    updateModel();

    createUniformBuffers();
    loaded = true;
}

void Model::createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout)
{
    auto &state = State::instance();
    auto &engine = state.engine;
    std::vector<vk::DescriptorSetLayout> layouts(engine.swapChain.count, layout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(engine.swapChain.count);
    allocInfo.pSetLayouts = layouts.data();

    colorSets = engine.device.create(allocInfo);

    if constexpr (Debug::enable)
    { // only do this if validation is enabled
        for (auto &descriptorSet : colorSets)
        {
            Debug::setName(engine.device.device, descriptorSet, name + " Color Set");
        }
    }

    for (size_t i = 0; i < engine.swapChain.count; ++i)
    {
        vk::DescriptorBufferInfo vertexInfo{};
        vertexInfo.buffer = vertBuffers[i].buffer;
        vertexInfo.offset = 0;
        vertexInfo.range = sizeof(UniformVert);

        vk::DescriptorBufferInfo fragInfo{};
        fragInfo.buffer = fragBuffers[i].buffer;
        fragInfo.offset = 0;
        fragInfo.range = sizeof(UniformFrag);

        vk::DescriptorImageInfo shadowInfo{};
        shadowInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        shadowInfo.imageView = state.scene.shadow.imageView;
        shadowInfo.sampler = state.scene.shadow.sampler;

        vk::DescriptorImageInfo diffuseInfo{};
        diffuseInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        diffuseInfo.imageView = material->diffuse.imageView;
        diffuseInfo.sampler = material->diffuse.sampler;

        vk::DescriptorImageInfo normalInfo{};
        normalInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        normalInfo.imageView = material->normal.imageView;
        normalInfo.sampler = material->normal.sampler;

        vk::DescriptorImageInfo roughnessInfo{};
        roughnessInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        roughnessInfo.imageView = material->roughness.imageView;
        roughnessInfo.sampler = material->roughness.sampler;

        vk::DescriptorImageInfo metallicInfo{};
        metallicInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        metallicInfo.imageView = material->metallic.imageView;
        metallicInfo.sampler = material->metallic.sampler;

        vk::DescriptorImageInfo aoInfo{};
        aoInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        aoInfo.imageView = material->ao.imageView;
        aoInfo.sampler = material->ao.sampler;

        vk::DescriptorImageInfo irradianceInfo{};
        irradianceInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        irradianceInfo.imageView = state.scene.backdrop->irradianceMap.imageView;
        irradianceInfo.sampler = state.scene.backdrop->irradianceMap.sampler;

        vk::DescriptorImageInfo radianceInfo{};
        radianceInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        radianceInfo.imageView = state.scene.backdrop->radianceMap.imageView;
        radianceInfo.sampler = state.scene.backdrop->radianceMap.sampler;

        vk::DescriptorImageInfo brdfInfo{};
        brdfInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        brdfInfo.imageView = state.scene.brdf.imageView;
        brdfInfo.sampler = state.scene.brdf.sampler;

        std::vector<vk::WriteDescriptorSet> descriptorWrites(11);

        // vert uniform buffer
        descriptorWrites[0].dstSet = colorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &vertexInfo;

        // frag uniform buffer
        descriptorWrites[1].dstSet = colorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &fragInfo;

        // shadow
        descriptorWrites[2].dstSet = colorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &shadowInfo;

        // diffuse
        descriptorWrites[3].dstSet = colorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &diffuseInfo;

        // normal
        descriptorWrites[4].dstSet = colorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &normalInfo;

        // roughness
        descriptorWrites[5].dstSet = colorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &roughnessInfo;

        // metallic
        descriptorWrites[6].dstSet = colorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &metallicInfo;

        // ao
        descriptorWrites[7].dstSet = colorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[7].descriptorCount = 1;
        descriptorWrites[7].pImageInfo = &aoInfo;

        // irradiance
        descriptorWrites[8].dstSet = colorSets[i];
        descriptorWrites[8].dstBinding = 8;
        descriptorWrites[8].dstArrayElement = 0;
        descriptorWrites[8].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[8].descriptorCount = 1;
        descriptorWrites[8].pImageInfo = &irradianceInfo;

        // radiance
        descriptorWrites[9].dstSet = colorSets[i];
        descriptorWrites[9].dstBinding = 9;
        descriptorWrites[9].dstArrayElement = 0;
        descriptorWrites[9].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[9].descriptorCount = 1;
        descriptorWrites[9].pImageInfo = &radianceInfo;

        // pregenned brdf sampler
        descriptorWrites[10].dstSet = colorSets[i];
        descriptorWrites[10].dstBinding = 10;
        descriptorWrites[10].dstArrayElement = 0;
        descriptorWrites[10].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[10].descriptorCount = 1;
        descriptorWrites[10].pImageInfo = &brdfInfo;

        engine.device.update(descriptorWrites);
    }
}

void Model::createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout)
{
    auto &engine = State::instance().engine;
    std::vector<vk::DescriptorSetLayout> layouts(engine.swapChain.count, layout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(engine.swapChain.count);
    allocInfo.pSetLayouts = layouts.data();

    shadowSets = engine.device.create(allocInfo);

    if constexpr (Debug::enable)
    { // only do this if validation is enabled
        for (auto &descriptorSet : shadowSets)
        {
            Debug::setName(engine.device.device, descriptorSet, name + " Shadow Set");
        }
    }

    for (size_t i = 0; i < engine.swapChain.count; ++i)
    {
        vk::DescriptorBufferInfo shadowInfo{};
        shadowInfo.buffer = shadBuffers[i].buffer;
        shadowInfo.offset = 0;
        shadowInfo.range = sizeof(UniformShad);

        std::vector<vk::WriteDescriptorSet> descriptorWrites(1);

        // shadow
        descriptorWrites[0].dstSet = shadowSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &shadowInfo;

        engine.device.update(descriptorWrites);
    }
}

void Model::createUniformBuffers()
{
    auto &count = State::instance().engine.swapChain.count;

    vertBuffers.resize(count);
    fragBuffers.resize(count);
    shadBuffers.resize(count);
    for (size_t i = 0; i < count; ++i)
    {
        vertBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        vertBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if constexpr (Debug::enable)
        {
            vertBuffers[i].name = fmt::format("Model {} Vert", name);
        }
        vertBuffers[i].create(sizeof(UniformVert));

        fragBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        fragBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if constexpr (Debug::enable)
        {
            fragBuffers[i].name = fmt::format("Model {} Frag", name);
        }
        fragBuffers[i].create(sizeof(UniformFrag));

        shadBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        shadBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if constexpr (Debug::enable)
        {
            shadBuffers[i].name = fmt::format("Model {} Shad", name);
        }
        shadBuffers[i].create(sizeof(UniformShad));
    }
}

} // namespace tat