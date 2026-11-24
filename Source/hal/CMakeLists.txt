add_library(gta2_hal STATIC
    HalContext.cpp
)
target_include_directories(gta2_hal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})