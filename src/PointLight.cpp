#include "PointLight.hpp"

namespace tat {

void PointLight::load() {
  name = config->name;
  light.position = config->position;
  light.temperature = config->temperature;
  light.lumens = config->lumens;
}

} // namespace tat