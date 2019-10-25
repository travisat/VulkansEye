#pragma once

#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.h>

namespace tat
{
struct Vertex
{
    glm::vec3 position;
    glm::vec2 UV;
    glm::vec3 normal;

    static auto getBindingDescription() -> VkVertexInputBindingDescription
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static auto getAttributeDescriptions() -> std::array<VkVertexInputAttributeDescription, 3>
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDesriptions = {};
        attributeDesriptions[0].binding = 0;
        attributeDesriptions[0].location = 0;
        attributeDesriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDesriptions[0].offset = offsetof(Vertex, position);

        attributeDesriptions[1].binding = 0;
        attributeDesriptions[1].location = 1;
        attributeDesriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDesriptions[1].offset = offsetof(Vertex, UV);

        attributeDesriptions[2].binding = 0;
        attributeDesriptions[2].location = 2;
        attributeDesriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
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