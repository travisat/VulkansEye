#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace tat
{

class Object
{
  public:
    auto position() -> glm::vec3
    {
        return m_position;
    };

  protected:
    glm::vec3 m_position = glm::vec3(0.F);
    glm::vec3 m_rotation = glm::vec3(0.F);
    glm::vec3 m_scale = glm::vec3(1.F);
    glm::vec3 m_velocity = glm::vec3(0.F);
    glm::vec3 m_acceleration = glm::vec3(0.F);
    glm::vec3 m_force = glm::vec3(0.F);
    glm::vec3 m_size = glm::vec3(0.F);

    float m_mass; // kg

  private:
};

} // namespace tat