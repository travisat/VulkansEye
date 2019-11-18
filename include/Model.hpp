#pragma once

#include <algorithm>
#include <memory>

#include "Buffer.hpp"
#include "Collection.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Object.hpp"

namespace tat
{

struct UniformVert
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 lightMVP;
    glm::mat4 normalMatrix;
    glm::vec4 camPos;
};
struct UniformFrag
{
    glm::vec4 position;
    float radianceMipLevels;
    float shadowSize;
};

struct UniformShad
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

class Model : public Object, public Entry
{
  public:
    void load() override;

    virtual ~Model() = default;

    Image *irradianceMap;
    Image *radianceMap;

    std::vector<vk::DescriptorSet> colorSets;
    std::vector<Buffer> vertBuffers;
    std::vector<Buffer> fragBuffers;
    std::vector<vk::DescriptorSet> shadowSets;
    std::vector<Buffer> shadBuffers;

    void createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createUniformBuffers();

    inline auto getMesh() -> Mesh *
    {
        return mesh;
    };

  private:
    Material *material;
    Mesh *mesh;
};

} // namespace tat