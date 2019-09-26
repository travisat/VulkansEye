#pragma once

#include "Helpers.h"
#include "Image.h"
#include "Buffer.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

static std::vector<Vertex> vertices = {
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}}, // -X side
    {{-1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},

    {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}}, // -Z side
    {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
    {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
    {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}},
    {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},

    {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}}, // -Y side}
    {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
    {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},

    {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}}, // +Y side}
    {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},

    {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}}, // +X side
    {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
    {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
    {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}},

    {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // +Z side
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
    {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}};

class Skybox
{
public:
    Skybox::Skybox(State *_state, Camera *_camera, std::string _texturePath)
        : state(_state), camera(_camera), texturePath(_texturePath){};

    ~Skybox();

    void create();
    void cleanup();
    void recreate();

    void updateUniformBuffer(uint32_t currentImage);

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<Buffer *> uniformBuffers;
    
    Buffer* vertexBuffer;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

private:
    State *state;
    Image *cubeMap;
    VkSampler sampler;

    VkDescriptorSetLayout descriptorSetLayout;
    std::string texturePath;

    Camera *camera;

    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorSets();
    void createPipeline();
};