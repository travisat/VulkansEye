#include "Object.hpp"
#include "glm/gtx/string_cast.hpp"
namespace tat
{

void Object::update(float deltaTime)
{
    // mass of 0 means don't move
    if (m_mass > 0.F)
    {
        // zero out force if its really low
        // this is to avoid floating point errors
        if (glm::abs(m_force.x) < 0.0001F)
        {
            m_force.x = 0.F;
        }
        if (glm::abs(m_force.y) < 0.0001F)
        {
            m_force.y = 0.F;
        }
        if (glm::abs(m_force.z) < 0.0001F)
        {
            m_force.z = 0.F;
        }

        // zero out velocity if its really low
        if (glm::abs(m_velocity.x) < 0.0001F)
        {
            m_velocity.x = 0.F;
        }
        if (glm::abs(m_velocity.y) < 0.0001F)
        {
            m_velocity.y = 0.F;
        }
        if (glm::abs(m_velocity.z) < 0.0001F)
        {
            m_velocity.z = 0.F;
        }

        // calculate acceleration
        m_acceleration = deltaTime * m_force / m_mass;

        // clear force since its been applied
        m_force = glm::vec3(0.F);

        // apply gravity
        m_acceleration.y += 9.8F;

        // apply acceleration to velocity
        m_velocity += m_acceleration * deltaTime;
        // apply velocity to position
        auto deltaPosition = m_velocity * deltaTime;

        translate(deltaPosition);
        // move object y back to 0 if lower than 0
        // zero out velocity
        // TODO(travis) replace this with collision detection
        auto bottom = m_position.y + m_size.y/2;
        if (bottom > 0.F)
        {
            auto diff = glm::vec3(0.F, -bottom, 0.F);
            translate(diff);
            m_velocity.y = 0.F;
        }

        updateView();
        updateModel();
    }
}

void Object::applyForce(glm::vec3 force)
{
    m_force += force;
}

void Object::translate()
{
    T = glm::translate(glm::mat4(1.F), m_position);
}

void Object::translate(glm::vec3 translation)
{
    m_position += translation;
    translate();
}

void Object::rotate()
{
    R = glm::rotate(glm::mat4(1.F), glm::radians(m_rotation.x), glm::vec3(1.F, 0.F, 0.F));
    R = glm::rotate(R, glm::radians(m_rotation.y), glm::vec3(0.F, 1.F, 0.F));
    R = glm::rotate(R, glm::radians(m_rotation.z), glm::vec3(0.F, 0.F, 1.F));
}

void Object::rotate(glm::vec3 rotation)
{
    m_rotation += rotation;
    rotate();
}

void Object::scale(glm::vec3 scale)
{
    m_scale *= scale;
    S = glm::scale(glm::mat4(1.F), m_scale);
    m_size = m_scale * m_size;
}

void Object::updateModel()
{
    M = T * R * S;
}

void Object::updateModel(glm::mat4 model)
{
    M = model;
}

void Object::updateView()
{
    V = R * T;
}

void Object::updateView(glm::mat4 view)
{
    V = view;
}

void Object::updateProjection(glm::mat4 projection)
{
    P = projection;
}

} // namespace tat