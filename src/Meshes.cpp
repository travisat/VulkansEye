#include "Meshes.hpp"
#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/vector2.h"
#include "assimp/vector3.h"
#include "helpers.h"
#include "vulkan/vulkan.hpp"

namespace tat
{
void Meshes::loadConfig(const MeshesConfig &config)
{

    configs.resize(config.meshes.size());
    collection.resize(configs.size());
    int32_t index = 0;
    for (const auto &meshConfig : config.meshes)
    {
        collection[index].name = meshConfig.name;
        // insert name into map for index retrieval
        names.insert(std::make_pair(meshConfig.name, index));
        // insert config into configs so mesh can be loaded when needed
        configs[index] = meshConfig;
        ++index;
    }
}

auto Meshes::getMesh(const std::string &name) -> Mesh *
{
    // get index, returns 0 for default if name of mesh not found
    int32_t index = getIndex(name);
    // load mesh if not loaded
    loadMesh(index);
    // return mesh
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
    importMesh(configs[index].path, mesh);

    // copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

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

void Meshes::importMesh(const std::string &path, Mesh *mesh)
{
    Assimp::Importer importer;
    uint32_t processFlags = aiProcess_GenSmoothNormals | aiProcess_FixInfacingNormals | aiProcess_GenUVCoords |
                            aiProcess_Triangulate | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes |
                            aiProcess_JoinIdenticalVertices;
    const aiScene *pScene = importer.ReadFile(path, processFlags);

    assert(pScene); // TODO(travis) error handling

    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        const aiMesh *aimesh = pScene->mMeshes[i];
        for (int j = 0; j < aimesh->mNumFaces; ++j)
        {
            const aiFace *face = &aimesh->mFaces[j];
            for (int k = 0; k < face->mNumIndices; ++k)
            {
                mesh->indices.push_back(face->mIndices[k]);
            }
        }
        for (int j = 0; j < aimesh->mNumVertices; ++j)
        {
            Vertex vertex{};
            vertex.position.x = aimesh->mVertices[j].x;
            vertex.position.y = aimesh->mVertices[j].y;
            vertex.position.z = aimesh->mVertices[j].z;
            vertex.UV.x = aimesh->mTextureCoords[0][j].x;
            vertex.UV.y = aimesh->mTextureCoords[0][j].y;
            vertex.normal.x = aimesh->mNormals[j].x;
            vertex.normal.y = aimesh->mNormals[j].y;
            vertex.normal.z = aimesh->mNormals[j].z;
            mesh->vertices.push_back(vertex);
        }
    }
    mesh->indexSize = mesh->indices.size();
    assert(mesh->indexSize > 0);
    mesh->vertexSize = mesh->vertices.size();
    assert(mesh->vertexSize > 0);
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