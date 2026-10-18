cmake_minimum_required(VERSION 3.20)
project(gta6_fake_menu_ps5sdk C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# PS5SDK root
if(NOT DEFINED ENV{PS5SDK})
    message(FATAL_ERROR "PS5SDK environment variable not set")
endif()
set(PS5SDK $ENV{PS5SDK})

# Firmware version (4.51 is the highest with offsets in PS5SDK)
if(NOT DEFINED ENV{PS5SDK_FW})
    set(PS5SDK_FW "0x451")
else()
    set(PS5SDK_FW $ENV{PS5SDK_FW})
endif()

# Toolchain
include(${PS5SDK}/cmake/toolchain-ps5.cmake)

# Output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include paths
include_directories(SYSTEM ${PS5SDK})
include_directories(SYSTEM ${PS5SDK}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.c
    src/menu.c
    src/render.c
    src/input.c
    src/audio.c
    src/assets.c
    src/sony_resolve.c
)

# Link against PS5SDK CRT + embedded assets
add_executable(gta6_fake_menu ${SOURCES}
    assets/raw/embed/logo.o
    assets/raw/embed/landscape.o
    assets/raw/embed/theme.o
)
target_link_libraries(gta6_fake_menu PRIVATE ${PS5SDK}/lib/libps5sdk_crt.a)

# Linker flags from PS5SDK toolchain
set_target_properties(gta6_fake_menu PROPERTIES
    LINK_FLAGS "-fuse-ld=lld -Xlinker -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.x -Wl,--build-id=none"
)

# Copy linker script for CMake compiler test
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/linker.x ${CMAKE_BINARY_DIR}/linker.x COPYONLY)