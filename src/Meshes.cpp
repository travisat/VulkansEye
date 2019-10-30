#include "Meshes.hpp"
#include "helpers.h"
#include "vulkan/vulkan.hpp"

namespace tat
{
void Meshes::loadConfigs(const std::vector<MeshConfig> &meshes)
{
    
    // resize and allow for 0 index to be default
    configs.resize(meshes.size() + 1);
    collection.resize(configs.size());
    int32_t index = 1; // start at 1 because index of 0 is reserved for default
    for (const auto& meshConfig : meshes)
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
    loadObj(configs[index].path, collection[index].vertices, collection[index].indices);
    collection[index].vertexSize = static_cast<uint32_t>(collection[index].vertices.size());
    collection[index].indexSize = static_cast<uint32_t>(collection[index].indices.size());
    
    // copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    stagingBuffer.update(collection[index].vertices);
    collection[index].vertexBuffer.vulkan = vulkan;
    collection[index].vertexBuffer.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    collection[index].vertexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(collection[index].vertexBuffer);

    stagingBuffer.update(collection[index].indices);
    collection[index].indexBuffer.vulkan = vulkan;
    collection[index].indexBuffer.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    collection[index].indexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(collection[index].indexBuffer);

    Trace("Loaded ", configs[index].path, " at ", Timer::systemTime());

    mesh->loaded = true;
}


void Meshes::loadObj(const std::string &path, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
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