#include "Meshes.hpp"
#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/vector2.h"
#include "assimp/vector3.h"
#include "helpers.h"
#include "vulkan/vulkan.hpp"

namespace tat
{
void Meshes::loadConfig(const MeshesConfig &config)
{
    collection.resize(config.meshes.size());
    int32_t index = 0;
    for (const auto &meshConfig : config.meshes)
    {
        collection[index].name = meshConfig.name;
        collection[index].path = meshConfig.path;
        collection[index].center = meshConfig.center;
        collection[index].size = meshConfig.size;
        // insert name into map for index retrieval
        names.insert(std::make_pair(meshConfig.name, index));
        // insert config into configs so mesh can be loaded when needed
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
    importMesh(mesh);

    // copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    stagingBuffer.update(mesh->data.vertices.data(), mesh->data.vertices.size() * sizeof(mesh->data.vertices[0]));
    mesh->buffers.vertex.vulkan = vulkan;
    mesh->buffers.vertex.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    mesh->buffers.vertex.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(mesh->buffers.vertex);

    stagingBuffer.update(mesh->data.indices.data(), mesh->data.indices.size() * sizeof(mesh->data.indices[0]));
    mesh->buffers.index.vulkan = vulkan;
    mesh->buffers.index.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    mesh->buffers.index.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(mesh->buffers.index);

    mesh->loaded = true;
    Trace("Loaded ", mesh->name, " at ", Timer::systemTime());
    Trace("Vertices: ", mesh->data.vertices.size());
    Trace("Indices: ", mesh->data.indices.size());
}

void Meshes::importMesh(Mesh *mesh)
{
    Assimp::Importer importer;
    auto processFlags = aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_PreTransformVertices |
                            aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded;
    auto pScene = importer.ReadFile(mesh->path, processFlags);

    assert(pScene); // TODO(travis) error handling

    const aiVector3D zero3D(0.F, 0.F, 0.F);

    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        auto aimesh = pScene->mMeshes[i];
        
        for (int j = 0; j < aimesh->mNumFaces; ++j)
        {
            const aiFace *face = &aimesh->mFaces[j];
            for (int k = 0; k < face->mNumIndices; ++k)
            {
                mesh->data.indices.push_back(face->mIndices[k]);
            }
        }

        for (int j = 0; j < aimesh->mNumVertices; ++j)
        {
            auto pPosition = &aimesh->mVertices[j];
            auto pUV = aimesh->HasTextureCoords(0) ? &aimesh->mTextureCoords[0][j] : &zero3D;
            auto pNormal = &aimesh->mNormals[j];
            Vertex vertex{};
            vertex.position.x = pPosition->x;
            vertex.position.y = pPosition->y;
            vertex.position.z = pPosition->z;
            vertex.UV.x = pUV->x;
            vertex.UV.y = pUV->y;
            vertex.normal.x = pNormal->x;
            vertex.normal.y = pNormal->y;
            vertex.normal.z = pNormal->z;
            mesh->data.vertices.push_back(vertex);
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