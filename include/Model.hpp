#pragma once

#include "Buffer.hpp"
#include "Collection.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Object.hpp"
#include <memory>

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

    std::shared_ptr<Image> irradianceMap;
    std::shared_ptr<Image> radianceMap;

    std::vector<vk::DescriptorSet> colorSets;
    std::vector<Buffer> vertBuffers;
    std::vector<Buffer> fragBuffers;
    std::vector<vk::DescriptorSet> shadowSets;
    std::vector<Buffer> shadBuffers;

    void createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    void createUniformBuffers();

    inline auto getMesh() -> std::shared_ptr<Mesh>
    {
        return mesh;
    };

  private:
    std::shared_ptr<Material> material;
    std::shared_ptr<Mesh> mesh;
};

} // namespace tat