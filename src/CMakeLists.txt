add_executable(${PROJECT_NAME} main.c emulator/apu.c emulator/cartridge.c emulator/cpu.c emulator/memory.c emulator/ppu.c emulator/timer.c)
#target_link_libraries(${PROJECT_NAME} PRIVATE vendor)



option(PLATFORM "Target platform" "SDL")


#if(PLATFORM STREQUAL "SDL")
    add_library(platform platform/sdl_platform.c)
    target_link_libraries(platform PRIVATE SDL3::SDL3)
    
#endif()
target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/platform)
target_link_libraries(${PROJECT_NAME} PRIVATE platform)
