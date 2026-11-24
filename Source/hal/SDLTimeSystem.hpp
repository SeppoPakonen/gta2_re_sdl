#pragma once

#include "ITimeSystem.hpp"

namespace gta2::hal {

class SDLTimeSystem : public ITimeSystem {
  public:
    u64 GetTicksMs() override;
    u64 GetTicksMicroseconds() override;
    void SleepMs(u32 milliseconds) override;
};

} // namespace gta2::hal