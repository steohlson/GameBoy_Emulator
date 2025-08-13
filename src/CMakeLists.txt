add_executable(${PROJECT_NAME} main.c )
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)

option(PLATFORM "Target platform" "SDL")

if(PLATFORM STREQUAL "SDL")
    add_library(platform src/platform/sdl/sdl_platform.c)
    target_link_libraries(${PROJECT_NAME} PRIVATE sdl)
endif()