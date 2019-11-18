#include "engine/Fence.hpp"
#include "State.hpp"

namespace tat
{

Fence::Fence()
{
    auto &device = State::instance().engine.device;
    vk::FenceCreateInfo fenceInfo {};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    fence = device.createFence(fenceInfo);
}

Fence::~Fence()
{
    if (fence)
    {
        auto &device = State::instance().engine.device;
        device.destroyFence(fence);
    }
}

} // namespace tat