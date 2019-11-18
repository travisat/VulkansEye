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
    virtual ~Material();

    Image diffuse;
    Image normal;
    Image metallic;
    Image roughness;
    Image ao;

  private:
    void loadImage(const std::string &file, Image *image);
};

} // namespace tat