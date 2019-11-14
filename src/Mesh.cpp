#include <memory>
#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>

#include "Config.hpp"
#include "Mesh.hpp"
#include "State.hpp"
#include "spdlog/spdlog.h"

namespace tat
{

void Mesh::load()
{
    auto& mesh = State::instance().at("meshes").at(name);
    path = mesh["path"];
    size.x = mesh["size"]["x"];
    size.y = mesh["size"]["y"];
    size.z = mesh["size"]["z"];
    
    import();

    // copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.flags = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    stagingBuffer.update(data.vertices.data(), data.vertices.size() * sizeof(data.vertices[0]));
    buffers.vertex.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    buffers.vertex.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(buffers.vertex);

    stagingBuffer.update(data.indices.data(), data.indices.size() * sizeof(data.indices[0]));
    buffers.index.flags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    buffers.index.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(buffers.index);

    loaded = true;
    spdlog::get("debugLogger")->info("Loaded Mesh {}", name);
}

void Mesh::import()
{
    Assimp::Importer importer;
    auto processFlags = aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_PreTransformVertices |
                        aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded;
    auto pScene = importer.ReadFile(path, processFlags);

    const aiVector3D zero3D(0.F, 0.F, 0.F);

    if (pScene == nullptr)
    {
        spdlog::get("debugLogger")->error("Unable to load {}", path);
        throw std::runtime_error("Unable to load mesh");
    }

    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        auto aimesh = pScene->mMeshes[i];

        for (int j = 0; j < aimesh->mNumFaces; ++j)
        {
            const aiFace *face = &aimesh->mFaces[j];
            for (int k = 0; k < face->mNumIndices; ++k)
            {
                data.indices.push_back(face->mIndices[k]);
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
            data.vertices.push_back(vertex);
        }
    }
}

} // namespace tat