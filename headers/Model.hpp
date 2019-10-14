#pragma once

#include <unordered_map>

#include <tiny_obj_loader.h>

#include "Vertex.h"
#include "Config.h"
#include "Timer.h"

#include "Image.hpp"

namespace tat
{

static void loadObj(std::string path, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
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
                (attrib.vertices[3 * index.vertex_index + 0]),
                (attrib.vertices[3 * index.vertex_index + 1]),
                (attrib.vertices[3 * index.vertex_index + 2])};
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
    Image metallic;
    Image ao;
    Image displacement;
    VkSampler diffuseSampler;
    VkSampler normalSampler;
    VkSampler roughnessSampler;
    VkSampler metallicSampler;
    VkSampler aoSampler;
    VkSampler dispSampler;

    ~Model();
    void create();

private:
    void loadMesh();
    void loadMaterial();
    void loadImage(const std::string &path, Image &image, VkSampler &sampler);
};

} //namespace tat