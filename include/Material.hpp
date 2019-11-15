#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Image.hpp"
#include "Collection.hpp"

namespace tat
{

class Material : public Entry
{
  public:
    void load() override;
    virtual ~Material() = default;

    std::shared_ptr<Image> diffuse = nullptr;
    std::shared_ptr<Image> normal = nullptr;
    std::shared_ptr<Image> metallic = nullptr;
    std::shared_ptr<Image> roughness = nullptr;
    std::shared_ptr<Image> ao = nullptr;

  private:
    auto loadImage(const std::string &file) -> std::shared_ptr<Image>;
};

} // namespace tat