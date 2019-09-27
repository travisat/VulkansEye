#pragma once

#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "Helpers.h"

struct UniformBufferObject
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    glm::vec3 cameraPosition;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texCoord;
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
        attributeDesriptions[0].offset = offsetof(Vertex, pos);

        attributeDesriptions[1].binding = 0;
        attributeDesriptions[1].location = 1;
        attributeDesriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDesriptions[1].offset = offsetof(Vertex, texCoord);

        attributeDesriptions[2].binding = 0;
        attributeDesriptions[2].location = 2;
        attributeDesriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDesriptions[2].offset = offsetof(Vertex, normal);
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
                hash<glm::vec2>()(vertex.texCoord) ^
                hash<glm::vec3>()(vertex.normal) << 1);
    }
};
} // namespace std