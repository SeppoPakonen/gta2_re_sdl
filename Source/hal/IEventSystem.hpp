#pragma once
#include "HalTypes.hpp"

namespace gta2::hal {

class IEventSystem {
public:
    virtual ~IEventSystem() = default;

    virtual bool PollEvent(Event& event) = 0;
    virtual void PumpEvents() = 0;
    virtual bool ShouldQuit() const = 0;
    virtual void RequestQuit() = 0;
};

} // namespace gta2::hal