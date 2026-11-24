#include "ProfilerTimer.hpp"
#include "Function.hpp"
#if !defined(GTA2_MODERN_BUILD)
#include <windows.h>
#else
#include "hal/HalContext.hpp"
#endif

MATCH_FUNC(0x5BEBF0)
void ProfilerTimer::AccumulateElapsed_5BEBF0()
{
#if !defined(GTA2_MODERN_BUILD)
    field_8_accum_time += timeGetTime() - field_4_start_time;
#else
    u32 now = gta2::hal::GetHal().pTime ? gta2::hal::GetHal().pTime->GetTicksMs() : 0;
    field_8_accum_time += now - field_4_start_time;
#endif
}

MATCH_FUNC(0x5BEC10)
ProfilerTimer::ProfilerTimer()
{
    field_0_time_percent = 0;
    field_8_accum_time = 0;
    field_4_start_time = 0;
}

MATCH_FUNC(0x5BEC20)
ProfilerTimer::~ProfilerTimer()
{
}