#pragma once

#include <unordered_map>

#include <tiny_obj_loader.h>

#include "Vertex.h"
#include "Config.h"
#include "Timer.h"

#include "Image.hpp"

static void loadObj(std::string path, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, glm::vec3 scale = glm::vec3(1.0f), glm::vec3 position = glm::vec3(0.0f))
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()));

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex = {};
            vertex.position = {
                (attrib.vertices[3 * index.vertex_index + 0] * scale.x) + position.x,
                (attrib.vertices[3 * index.vertex_index + 1] * scale.y) + position.y,
                (attrib.vertices[3 * index.vertex_index + 2] * scale.z) + position.z};
            vertex.UV = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
};

class Model
{
public:
    //config values
    tat::Vulkan *vulkan = nullptr;
    ModelConfig *config;
    std::string name = "Uknown Model";
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 position = glm::vec3(0.0f);

    //generated values
    //mesh properties
    uint32_t vertexSize = 0;
    uint32_t indexSize = 0;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    //material properties
    Image diffuse;
    Image normal;
    Image roughness;
    Image ambientOcclusion;
    VkSampler diffuseSampler;
    VkSampler normalSampler;
    VkSampler roughnessSampler;
    VkSampler ambientOcclusionSampler;

    ~Model();
    void create();

private:
    void loadMesh();
    void loadMaterial();
    void loadImage(const std::string &path, ImageType type, Image &image, VkFormat format, VkSampler &sampler);
};