#include "Meshes.hpp"
#include "helpers.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace tat
{
void Meshes::loadConfig(const MeshesConfig &config)
{
    
    configs.resize(config.meshes.size());
    collection.resize(configs.size());
    int32_t index = 0; 
    for (const auto& meshConfig : config.meshes)
    {
        collection[index].name = meshConfig.name;
        //insert name into map for index retrieval
        names.insert(std::make_pair(meshConfig.name, index));
        //insert config into configs so mesh can be loaded when needed
        configs[index] = meshConfig;
        ++index;
    }
}

auto Meshes::getMesh(const std::string &name) -> Mesh *
{
    //get index, returns 0 for default if name of mesh not found
    int32_t index = getIndex(name);
    //load mesh if not loaded
    loadMesh(index);
    //return mesh
    return &collection[index];
}

void Meshes::loadMesh(int32_t index)
{
    // get pointer to mesh
    Mesh *mesh = &collection[index];
    // if already loaded return
    if (mesh->loaded == true)
    {
        return;
    }
    // otherwise load the mesh
    loadObj(configs[index].path, mesh);
    mesh->vertexSize = static_cast<uint32_t>(mesh->vertices.size());
    mesh->indexSize = static_cast<uint32_t>(mesh->indices.size());
    
    // copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    Trace(mesh->vertexSize);

    stagingBuffer.update(mesh->vertices.data(), mesh->vertexSize * sizeof(mesh->vertices[0]));
    mesh->vertexBuffer.vulkan = vulkan;
    mesh->vertexBuffer.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    mesh->vertexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(mesh->vertexBuffer);

    stagingBuffer.update(mesh->indices.data(), mesh->indexSize * sizeof(mesh->indices[0]));
    mesh->indexBuffer.vulkan = vulkan;
    mesh->indexBuffer.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    mesh->indexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(mesh->indexBuffer);

    mesh->loaded = true;
    Trace("Loaded ", configs[index].path, " at ", Timer::systemTime());
}


void Meshes::loadObj(const std::string &path, Mesh *mesh)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    std::unordered_map<Vertex, uint32_t> uniqueVertices {};

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
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh->vertices.size());
                mesh->vertices.emplace_back(vertex);
            }

            mesh->indices.emplace_back(uniqueVertices[vertex]);
        }
    }
}

auto Meshes::getIndex(const std::string &name) -> int32_t
{
    auto result = names.find(name);
    if (result == names.end())
    {
        // if name not in list return 0 index which is for default mesh
        return 0;
    }
    // otherwise return index for name
    return result->second;
}

} // namespace tat