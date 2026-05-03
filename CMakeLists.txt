# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Generate build version timestamp
string(TIMESTAMP BUILD_VERSION "%Y%m%d" UTC)
message(STATUS "Build version: ${BUILD_VERSION}")

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
        src/common/macro.c
        src/common/location.c
        src/common/currency.c
        src/common/error.c
)

# Display subsystem
set(DISPLAY_SOURCES
        src/display/matrix.c
        src/display/glyphs.c
)

# Input handling
set(INPUT_SOURCES
        src/input/input.c
)

# Menu system
set(MENU_SOURCES
        src/menu/menu_common.c
        src/menu/menu_main.c
        src/menu/menu_markets.c
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
        src/middleware/update.c
)

# Server (captive portal & config server)
set(SERVER_SOURCES
        src/server/dns_server.c
        src/server/http_server.c
        src/server/html.c
        src/server/dhcpserver.c
        src/server/parser.c
)

# mDNS sources from lwIP
set(MDNS_SOURCES
        ${PICO_LWIP_PATH}/src/apps/mdns/mdns.c
        ${PICO_LWIP_PATH}/src/apps/mdns/mdns_out.c
        ${PICO_LWIP_PATH}/src/apps/mdns/mdns_domain.c
)

# Main display modes
set(MAIN_MODE_SOURCES
        src/modes/weather.c
        src/modes/config.c
        src/modes/reset.c
)

# Markets display modes
set(MARKETS_MODE_SOURCES
        src/modes/markets/portfolio.c
        src/modes/markets/stock.c
        src/modes/markets/crypto.c
        src/modes/markets/currency_pair.c
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
        ${MAIN_MODE_SOURCES}
        ${MARKETS_MODE_SOURCES}
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
        $<$<COMPILE_LANGUAGE:C>:-include assert.h>
)

# Reserve last 256KB of flash for persistent user data that survives firmware updates
# Pico 2 W has 4MB flash: firmware uses 0x000000-0x3C0000 (3.75MB), config uses 0x3C0000-0x400000 (256KB)
set(PICO_FLASH_SIZE_BYTES 0x3C0000)  # 3.75MB for firmware (reserves last 256KB)

# lwIP configuration for larger TCP uploads (50 ticker portfolio support)
target_compile_definitions(photon PRIVATE
        TCP_WND=8192              # TCP receive window: 8KB (allows larger uploads)
        TCP_SND_BUF=8192          # TCP send buffer: 8KB
        MEMP_NUM_TCP_SEG=32       # Number of TCP segments
        PBUF_POOL_SIZE=24         # Packet buffer pool size
        BUILD_VERSION="${BUILD_VERSION}"  # Build timestamp for OTA updates
        PICO_FLASH_SIZE_BYTES=${PICO_FLASH_SIZE_BYTES}  # Limit firmware size to reserve flash
)

target_link_libraries(photon
        pico_stdlib)

target_include_directories(photon PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/common
        ${CMAKE_CURRENT_LIST_DIR}/src/display
        ${CMAKE_CURRENT_LIST_DIR}/src/input
        ${CMAKE_CURRENT_LIST_DIR}/src/menu
        ${CMAKE_CURRENT_LIST_DIR}/src/middleware
        ${CMAKE_CURRENT_LIST_DIR}/src/server
        ${CMAKE_CURRENT_LIST_DIR}/src/modes
        ${CMAKE_CURRENT_LIST_DIR}/src/modes/markets
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(photon
        hardware_pio
        hardware_flash
        hardware_sync
        hardware_adc
        pico_cyw43_arch_lwip_threadsafe_background
        pico_bootrom
        )

pico_add_extra_outputs(photon)
