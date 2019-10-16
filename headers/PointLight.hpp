#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Config.h"

namespace tat {

static const int numLights = 2;

struct uPointLight {
  glm::vec3 position;
  alignas(4) float lumens;
  alignas(4) float temperature;
  glm::vec3 buffer = glm::vec3(
      0.0f); // sizeof struct in uniform buffer must be divisible by 16
};

struct UniformLight {
  uPointLight light[numLights];
};

class PointLight {
public:
  PointLightConfig *config;

  uint32_t id;
  std::string name;
  uPointLight light{};
  void load();

private:
};

} // namespace tat