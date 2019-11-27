#pragma once

#include <array>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{
struct Vertex
{
    glm::vec3 position;
    glm::vec2 UV;
    glm::vec3 normal;

    static auto getBindingDescription() -> vk::VertexInputBindingDescription
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }

    static auto getAttributeDescriptions() -> std::array<vk::VertexInputAttributeDescription, 3>
    {
        std::array<vk::VertexInputAttributeDescription, 3> attributeDesriptions = {};
        attributeDesriptions[0].binding = 0;
        attributeDesriptions[0].location = 0;
        attributeDesriptions[0].format = vk::Format::eR32G32B32Sfloat;
        attributeDesriptions[0].offset = offsetof(Vertex, position);

        attributeDesriptions[1].binding = 0;
        attributeDesriptions[1].location = 1;
        attributeDesriptions[1].format = vk::Format::eR32G32Sfloat;
        attributeDesriptions[1].offset = offsetof(Vertex, UV);

        attributeDesriptions[2].binding = 0;
        attributeDesriptions[2].location = 2;
        attributeDesriptions[2].format = vk::Format::eR32G32B32Sfloat;
        attributeDesriptions[2].offset = offsetof(Vertex, normal);

        return attributeDesriptions;
    }

    auto operator==(const Vertex &other) const -> bool
    {
        return position == other.position && UV == other.UV && normal == other.normal;
    }
};

} // namespace tat

namespace std
{
template <> struct hash<tat::Vertex>
{
    auto operator()(tat::Vertex const &vertex) const -> size_t
    {
        return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.UV) ^
                hash<glm::vec3>()(vertex.normal) << 1);
    }
};
} // namespace std