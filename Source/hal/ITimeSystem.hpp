#pragma once
#include "HalTypes.hpp"

namespace gta2::hal {

class ITimeSystem {
public:
    virtual ~ITimeSystem() = default;

    virtual u64 GetTicksMs() = 0;
    virtual u64 GetTicksMicroseconds() = 0;
    virtual void SleepMs(u32 milliseconds) = 0;
};

} // namespace gta2::hal