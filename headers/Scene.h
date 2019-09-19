#pragma once
#include "Material.h"
#include "Model.h"
#include "Config.h"
#include "Helpers.h"
#include "Skybox.h"
#include "State.h"
#include "Input.h"

struct Object
{
    Object(){};
    ~Object()
    {
        for (auto buffer : uniformBuffers)
        {
            delete buffer;
        }
    };

    uint32_t materialIndex;
    uint32_t modelIndex;

    State *state;

    std::vector<Buffer *> uniformBuffers;
    std::vector<VkDescriptorSet> descriptorSets;

    double xpos;
    double ypos;
    double zpos;
};

class Scene
{
public:
    Scene(State *state, Config &config);
    ~Scene();
    void initScene();
    void initModels();
    void initMaterials();
    void initObjects();
    void loadObjects();
    void loadSkybox();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage);
    void createDescriptorSets();
    void createPipelines();

    VkPipeline getPipeline() { return pipeline; };
    VkPipelineLayout getPipelineLayout() { return pipelineLayout; };
    VkBuffer getVertexBuffer() { return vertexBuffer->buffer; };
    VkBuffer getIndexBuffer() { return indexBuffer->buffer; };
    Model *getModel(uint32_t i) { return models[i]; };
    Material *getMaterial(uint32_t i) { return materials[i]; };
    Object *getObject(uint32_t i) { return objects[i]; };
    uint32_t numObjects() { return static_cast<uint32_t>(objects.size()); };

    Skybox *skybox;

private:
    State *state;

    Buffer *vertexBuffer;
    Buffer *indexBuffer;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    std::vector<Object *> objects;
    std::vector<Model *> models;
    std::vector<Material *> materials;

    Config *config;
};