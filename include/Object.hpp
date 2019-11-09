#pragma once

#include "glm/fwd.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace tat
{

class Object
{
  public:
    void update(float deltaTime);
    void applyForce(glm::vec3 force);

    auto position() -> glm::vec3
    {
        return m_position;
    };

    auto rotation() -> glm::vec3
    {
        return m_rotation;
    }

    auto scale() -> glm::vec3
    {
        return m_scale;
    }

    auto model() -> glm::mat4
    {
        return M;
    };

    auto view() -> glm::mat4
    {
        return V;
    };

    auto projection() -> glm::mat4
    {
        return P;
    };

    void translate();
    void translate(glm::vec3 translation);
    void rotate();
    void rotate(glm::vec3 rotation);
    void scale(glm::vec3 scale);

    void updateModel(); // uses TRS to calculate M
    void updateModel(glm::mat4 model);
    void updateView(); // uses RT to calculate V
    void updateView(glm::mat4 view);
    void updateProjection(glm::mat4 projection);

  protected:
    glm::mat4 M; // model matrix
    glm::mat4 V; // view matrix
    glm::mat4 P; // projection matrix
    glm::mat4 T; // translation matrix
    glm::mat4 R; // rotation matrix
    glm::mat4 S; // scale matrix

    glm::vec3 m_position = glm::vec3(0.F);
    glm::vec3 m_rotation = glm::vec3(0.F);
    glm::vec3 m_scale = glm::vec3(1.F);
    glm::vec3 m_size = glm::vec3(0.F);
    glm::vec3 m_velocity = glm::vec3(0.F);
    glm::vec3 m_acceleration = glm::vec3(0.F);
    glm::vec3 m_force = glm::vec3(0.F);

    float m_mass = 0.F; // kg
};

} // namespace tat