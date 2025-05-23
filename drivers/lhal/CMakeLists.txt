sdk_generate_library()

sdk_library_add_sources(src/bflb_common.c)

sdk_library_add_sources(
src/bflb_cks.c
src/bflb_ef_ctrl.c
src/bflb_gpio.c
src/bflb_i2c.c
src/bflb_dma.c
src/bflb_rtc.c
src/bflb_sec_aes.c
src/bflb_sec_sha.c
src/bflb_sec_trng.c
src/bflb_spi.c
src/bflb_timer.c
src/bflb_uart.c
src/bflb_wdg.c
src/bflb_flash.c
src/flash/bflb_sf_cfg.c
src/flash/bflb_xip_sflash.c
src/flash/bflb_sflash.c
src/flash/bflb_sf_ctrl.c
src/flash/bflb_flash_secreg_port.c
src/flash/bflb_flash_secreg.c
)

if("${CHIP}" STREQUAL "bl602")
sdk_library_add_sources(
src/bflb_adc.c
src/bflb_acomp.c
src/bflb_dac.c
src/bflb_ir.c
src/bflb_pwm_v1.c
src/bflb_sdio2.c
)
elseif("${CHIP}" STREQUAL "bl702")
sdk_library_add_sources(
src/bflb_adc.c
src/bflb_acomp.c
src/bflb_dac.c
src/bflb_emac.c
src/bflb_ir.c
src/bflb_pwm_v1.c
src/bflb_cam.c
src/bflb_spi_psram.c
)
elseif("${CHIP}" STREQUAL "bl702l")
sdk_library_add_sources(
src/bflb_adc.c
src/bflb_acomp.c
src/bflb_kys.c
src/bflb_pwm_v1.c
src/bflb_pwm_v2.c
src/bflb_spi_psram.c
src/bflb_auadc.c
src/bflb_ir.c
)
elseif("${CHIP}" STREQUAL "bl616")
sdk_library_add_sources(
src/bflb_adc.c
src/bflb_acomp.c
src/bflb_dac.c
src/bflb_emac.c
src/bflb_ir.c
src/bflb_mjpeg.c
src/bflb_pwm_v2.c
src/bflb_cam.c
src/bflb_sdio2.c
src/bflb_i2s.c
src/bflb_dbi.c
src/bflb_audac.c
src/bflb_auadc.c
src/bflb_wo.c
src/bflb_sdh.c
)
elseif("${CHIP}" STREQUAL "bl808")
sdk_library_add_sources(
src/bflb_adc.c
src/bflb_acomp.c
src/bflb_dac.c
src/bflb_emac.c
src/bflb_ir.c
src/bflb_mjpeg.c
src/bflb_pwm_v2.c
src/bflb_cam.c
src/bflb_csi.c
src/bflb_dbi.c
src/bflb_i2s.c
src/bflb_wo.c
src/bflb_sdh.c
)
endif()

if(CONFIG_CHERRYUSB)
if("${CHIP}" STREQUAL "bl702")
sdk_library_add_sources(src/bflb_usb_v1.c)
elseif(("${CHIP}" STREQUAL "bl602") OR ("${CHIP}" STREQUAL "bl702l"))
# no usb
else()
sdk_library_add_sources(src/bflb_usb_v2.c)
endif()
endif()

# optional
sdk_library_add_sources(src/bflb_irq.c)
sdk_library_add_sources(src/bflb_l1c.c)
sdk_library_add_sources(src/bflb_mtimer.c)

sdk_add_include_directories(include)
sdk_add_include_directories(include/arch)
sdk_add_include_directories(include/arch/risc-v/t-head)
sdk_add_include_directories(include/arch/risc-v/t-head/Core/Include)
sdk_add_include_directories(include/arch/risc-v/t-head/csi_dsp/include)
sdk_add_include_directories(config/${CHIP})
sdk_add_include_directories(src/flash)

if((NOT ("${CHIP}" STREQUAL "bl702")) AND (NOT ("${CHIP}" STREQUAL "bl602")) AND (NOT ("${CHIP}" STREQUAL "bl702l")))
sdk_library_add_sources(include/arch/risc-v/t-head/rv_hart.c)
sdk_library_add_sources(include/arch/risc-v/t-head/rv_pmp.c)
if((NOT ("${CPU_ID}" STREQUAL "d0")) AND (NOT ("${CPU_ID}" STREQUAL "lp")))
sdk_add_static_library(include/arch/risc-v/t-head/csi_dsp/lib/libcsi_xt900p32f_dsp.a)
endif()
endif()

sdk_library_add_sources(config/${CHIP}/device_table.c)

string(TOUPPER ${CHIP} CHIPNAME)
sdk_add_compile_definitions(-D${CHIPNAME})

if(CPU_ID)
string(TOUPPER ${CPU_ID} CPU_ID_NAME)
sdk_add_compile_definitions(-DCPU_${CPU_ID_NAME})
endif()

if((NOT ("${CPU_ID}" STREQUAL "d0")) AND (NOT ("${CPU_ID}" STREQUAL "lp")))
sdk_add_static_library(src/pka/libpka_${CHIP}.a)
else()
# add_subdirectory(src/pka)
endif()

sdk_add_private_compile_definitions_ifdef(CONFIG_FLASH_2LINE -D${CONFIG_FLASH_2LINE})

if(CONFIG_LHAL_ROMAPI)
sdk_add_compile_definitions(-D CONFIG_LHAL_ROMAPI)
endif()

if(CONFIG_LHAL_BOOTROM_ROMDRIVER)
sdk_add_compile_definitions(-D CONFIG_LHAL_BOOTROM_ROMDRIVER)
sdk_library_add_sources(config/${CHIP}/bflb_lhal_romdriver.c)
endif()

# check VERSION file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    # read info from VERSION file
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION_FILE_CONTENT)

    string(REGEX MATCH "LHAL_VERSION[ ]+\"([^\"]+)\"" _ ${VERSION_FILE_CONTENT})
    set(LHAL_VERSION "${CMAKE_MATCH_1}")
else()
    # get git tag
    execute_process(
        COMMAND git describe --abbrev=8 --tags --dirty --always
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(GIT_TAG)
        set(LHAL_VERSION ${GIT_TAG})
    else()
        message(WARNING "No Valid version info found for LHAL!")
        set(LHAL_VERSION "version-unknown-panic")
    endif()
endif()
message(STATUS "LHAL Version: ${LHAL_VERSION}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config/lhal_version.h.in"
  "${CMAKE_BINARY_DIR}/generated/lhal_version.h"
)
sdk_add_include_directories("${CMAKE_BINARY_DIR}/generated")
sdk_library_add_sources(config/lhal_version.c)
