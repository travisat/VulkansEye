#include "engine/Semaphore.hpp"
#include "State.hpp"

namespace tat
{

Semaphore::Semaphore()
{
    auto &device = State::instance().engine.device;
    vk::SemaphoreCreateInfo createInfo {};
    semaphore = device.create(createInfo);
}

Semaphore::~Semaphore()
{
    if (semaphore)
    {
        auto &device = State::instance().engine.device;
        device.destroy(semaphore);
    }
}

} // namespace tat