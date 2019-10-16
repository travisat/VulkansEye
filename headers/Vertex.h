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


struct TessControl
{
    float tessLevel = 64.0f;
};

struct TessEval
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    float tessStrength = 0.1f;
    float tessAlpha = 0.3f;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec2 UV;
    glm::vec3 normal;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
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

    bool operator==(const Vertex &other) const
    {
        return position == other.position && UV == other.UV && normal == other.normal;
    }
};

} //namespace tat

namespace std
{
template <>
struct hash<tat::Vertex>
{
    size_t operator()(tat::Vertex const &vertex) const
    {
        return (hash<glm::vec3>()(vertex.position) ^
                hash<glm::vec2>()(vertex.UV) ^
                hash<glm::vec3>()(vertex.normal) << 1);
    }
};
} // namespace std