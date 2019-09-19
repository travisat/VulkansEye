#pragma once
#include "Helpers.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDesriptions = {};
        attributeDesriptions[0].binding = 0;
        attributeDesriptions[0].location = 0;
        attributeDesriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDesriptions[0].offset = offsetof(Vertex, pos);

        attributeDesriptions[1].binding = 0;
        attributeDesriptions[1].location = 1;
        attributeDesriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDesriptions[1].offset = offsetof(Vertex, texCoord);

        return attributeDesriptions;
    }

    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && texCoord == other.texCoord;
    }
};

namespace std
{
template <>
struct hash<Vertex>
{
    size_t operator()(Vertex const &vertex) const
    {
        return (hash<glm::vec3>()(vertex.pos) ^
               (hash<glm::vec2>()(vertex.texCoord) << 1));
    }
};
} // namespace std