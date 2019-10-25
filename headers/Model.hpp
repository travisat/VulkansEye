#pragma once

#include <cassert>
#include <unordered_map>

#include <tiny_obj_loader.h>


#include "Config.h"
#include "Timer.h"
#include "Vertex.h"

#include "Buffer.hpp"
#include "Image.hpp"

namespace tat
{

static void loadObj(const std::string& path, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()));

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex = {};
            vertex.position = {(attrib.vertices[3 * index.vertex_index + 0]),
                               (attrib.vertices[3 * index.vertex_index + 1]),
                               (attrib.vertices[3 * index.vertex_index + 2])};
            vertex.UV = {attrib.texcoords[2 * index.texcoord_index + 0],
                         1.0F - attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.normal = {attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
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
    // config values
    tat::Vulkan *vulkan = nullptr;
    ModelConfig *config;
    std::string name = "Uknown Model";

    // generated values
    // mesh properties
    glm::vec3 position = glm::vec3(0.0F);
    glm::vec3 rotation = glm::vec3(0.0F);
    glm::vec3 scale = glm::vec3(1.0F);

    uint32_t vertexSize = 0;
    uint32_t indexSize = 0;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    
    //color pipeline
    std::vector<VkDescriptorSet> colorSets;
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
    std::vector<Buffer> tescBuffers;
    std::vector<Buffer> teseBuffers;
    std::vector<Buffer> uniformLights;
    TessControl uTessControl = {};
    TessEval uTessEval = {};


    //shadow pipeline
    std::vector<VkDescriptorSet> shadowSets;
    Image *shadow;
    VkSampler shadowSampler;
    Buffer shadowBuffer;
    Buffer uniformBuffer;
    shadowTransforms uShadow = {};
    UniformBuffer uBuffer = {};

    ~Model();
    void create();
    void createColorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void createShadowSets(VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void createUniformBuffers();

  private:
    void loadMesh();
    void loadMaterial();
    void loadImage(const std::string &path, Image &image, VkSampler &sampler);
};

} // namespace tat