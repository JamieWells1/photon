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
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(photon C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Core sources
set(CORE_SOURCES
        main.c
)

# Common utilities
set(COMMON_SOURCES
        src/common/const.c
        src/common/macro.c
        src/common/location.c
        src/common/currency.c
        src/common/error.c
)

# Display subsystem
set(DISPLAY_SOURCES
        src/display/graphics.c
        src/display/matrix.c
        src/display/glyphs.c
)

# Input handling
set(INPUT_SOURCES
        src/input/input.c
)

# Menu system
set(MENU_SOURCES
        src/menu_common.c
        src/menu_main.c
        src/menu_markets.c
)

# Global state
set(GLOBAL_SOURCES
        src/global.c
)

# Utils
set(UTILS_SOURCES
        src/utils.c
)

# Power
set(POWER_SOURCES
        src/power.c
)

# Middleware (client-side networking & storage)
set(MIDDLEWARE_SOURCES
        src/middleware/wifi.c
        src/middleware/http.c
        src/middleware/flash_memory.c
)

# Server (captive portal & config server)
set(SERVER_SOURCES
        src/server/dns_server.c
        src/server/http_server.c
        src/server/html.c
        src/server/dhcpserver.c
)

# mDNS sources from lwIP
set(MDNS_SOURCES
        ${PICO_LWIP_PATH}/src/apps/mdns/mdns.c
        ${PICO_LWIP_PATH}/src/apps/mdns/mdns_out.c
        ${PICO_LWIP_PATH}/src/apps/mdns/mdns_domain.c
)

# Display modes
set(MODE_SOURCES
        src/modes/portfolio.c
        src/modes/stock.c
        src/modes/crypto.c
        src/modes/weather.c
        src/modes/config.c
)

add_executable(photon
        ${CORE_SOURCES}
        ${COMMON_SOURCES}
        ${DISPLAY_SOURCES}
        ${INPUT_SOURCES}
        ${MENU_SOURCES}
        ${GLOBAL_SOURCES}
        ${UTILS_SOURCES}
        ${POWER_SOURCES}
        ${MIDDLEWARE_SOURCES}
        ${SERVER_SOURCES}
        ${MDNS_SOURCES}
        ${MODE_SOURCES}
)

pico_set_program_name(photon "photon")
pico_set_program_version(photon "0.1")

pico_generate_pio_header(photon ${CMAKE_CURRENT_LIST_DIR}/pio/ws2812.pio)

pico_enable_stdio_uart(photon 0)
pico_enable_stdio_usb(photon 1)

# Enable strict compiler warnings to catch bugs early
target_compile_options(photon PRIVATE
        -Wall
        -Wextra
        -Werror=return-type
        -Wno-unused-parameter
        -Wno-type-limits
)

target_link_libraries(photon
        pico_stdlib)

target_include_directories(photon PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/common
        ${CMAKE_CURRENT_LIST_DIR}/src/display
        ${CMAKE_CURRENT_LIST_DIR}/src/input
        ${CMAKE_CURRENT_LIST_DIR}/src/middleware
        ${CMAKE_CURRENT_LIST_DIR}/src/server
        ${CMAKE_CURRENT_LIST_DIR}/src/modes
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(photon
        hardware_pio
        hardware_flash
        hardware_sync
        hardware_adc
        pico_cyw43_arch_lwip_threadsafe_background
        )

pico_add_extra_outputs(photon)
