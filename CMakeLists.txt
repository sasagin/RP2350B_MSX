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
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Set board to Pico 2 (RP2350)
set(PICO_BOARD weact_studio_rp2350b_core CACHE STRING "Board type")

# USB HID gamepad/keyboard support (enabled by default)
option(USB_HID_ENABLED "Enable USB HID host for gamepads and keyboards" ON)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(RP2350_MSX C CXX ASM)
SET(BUILD_NAME "${PROJECT_NAME}")


# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(${PROJECT_NAME} main.c)

pico_set_program_name(RP2350_MSX "RP2350Tester")
pico_set_program_version(RP2350_MSX "0.1")

add_subdirectory(drivers/SSG_AY)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME} PRIVATE
            pico_stdlib
            tinyusb_host
            tinyusb_device         
        )

    pico_enable_stdio_uart(${PROJECT_NAME} 1)
    pico_enable_stdio_usb(${PROJECT_NAME} 0)  
    
pico_generate_pio_header(${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/VDP_int_Z80.pio
        OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/
)
# Включаем поддержку USB Host в настройках
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUSB_RHPORT0_MODE=OPT_MODE_HOST
)
# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)  

# Add any user requested libraries
target_link_libraries(${PROJECT_NAME} PRIVATE

        aySoundSoft
     
        pico_runtime
        pico_stdlib
        hardware_vreg
        hardware_clocks
        hardware_pwm
        hardware_pio
        # hardware_i2c
        pico_multicore
        hardware_interp hardware_timer hardware_clocks hardware_pwm hardware_flash
        )

pico_add_extra_outputs(${PROJECT_NAME})

