#include "engine/Semaphore.hpp"
#include "State.hpp"

namespace tat
{

Semaphore::Semaphore()
{
    auto &device = State::instance().engine.device;
    semaphore = device.createSemaphore();
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