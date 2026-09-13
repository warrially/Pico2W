# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(switch2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(switch2
    switch2.c
    usb_descriptors.c
    wifi.c
    http_server.c
)

pico_set_program_name(switch2 "switch2")
pico_set_program_version(switch2 "0.1")

# USB作为HID手柄使用，因此关闭stdio USB/UART
pico_enable_stdio_uart(switch2 0)
pico_enable_stdio_usb(switch2 0)

# 当前工程目录
target_include_directories(switch2 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Pico SDK + TinyUSB + Wi-Fi
target_link_libraries(switch2
    pico_stdlib

    tinyusb_device
    tinyusb_board

    pico_cyw43_arch_lwip_threadsafe_background
)

pico_add_extra_outputs(switch2)