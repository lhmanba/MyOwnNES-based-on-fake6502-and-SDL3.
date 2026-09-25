cmake_minimum_required(VERSION 3.21)

project(minines VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)

add_executable(minines 
    src/main.c 
    src/cartridge.c 
    src/mapper0.c
    src/bus.c
    src/cpu6502_adapter.c
    src/ppu.c
    src/nes.c
    src/platform_sdl.c
    src/controller.c
)

if(MSVC)
    target_compile_options(minines PRIVATE /W4)
    add_compile_options(/utf-8)
else()
    target_compile_options(minines PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(minines PRIVATE SDL3::SDL3)
target_include_directories(minines PRIVATE 
    src
    third_party/MyLittle6502-main
)
target_compile_definitions(minines PRIVATE
    FAKE6502_USE_STDINT
    NES_CPU
)

add_custom_command(TARGET minines POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:minines>
)