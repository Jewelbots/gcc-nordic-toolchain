TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

# application source
C_SOURCE_FILES += main.c
C_SOURCE_FILES += softdevice_handler.c
C_SOURCE_FILES += softdevice_handler_appsh.c
C_SOURCE_FILES += client_handling.c
C_SOURCE_FILES += ble_advdata_parser.c
C_SOURCE_FILES += ble_advdata.c
C_SOURCE_FILES += ble_advertising.c
C_SOURCE_FILES += ble_bas.c
C_SOURCE_FILES += ble_conn_params.c
C_SOURCE_FILES += ble_db_discovery.c
C_SOURCE_FILES += ble_debug_assert_handler.c
C_SOURCE_FILES += ble_dis.c
C_SOURCE_FILES += ble_dtm.c
C_SOURCE_FILES += ble_error_log.c
C_SOURCE_FILES += ble_flash.c
C_SOURCE_FILES += ble_nus.c
C_SOURCE_FILES += ble_racp.c
C_SOURCE_FILES += ble_radio_notification.c
C_SOURCE_FILES += ble_srv_common.c
C_SOURCE_FILES += ble_dfu.c
C_SOURCE_FILES += device_manager_central.c
C_SOURCE_FILES += app_uart.c
C_SOURCE_FILES += nrf_adc.c
C_SOURCE_FILES += nrf_delay.c
C_SOURCE_FILES += nrf_drv_clock.c
C_SOURCE_FILES += nrf_drv_common.c
C_SOURCE_FILES += nrf_drv_gpiote.c
C_SOURCE_FILES += nrf_drv_lpcomp.c
C_SOURCE_FILES += nrf_drv_ppi.c
C_SOURCE_FILES += nrf_drv_qdec.c
C_SOURCE_FILES += nrf_drv_rng.c
C_SOURCE_FILES += nrf_drv_rtc.c
C_SOURCE_FILES += nrf_drv_timer.c
C_SOURCE_FILES += nrf_drv_twi.c
C_SOURCE_FILES += nrf_drv_wdt.c
C_SOURCE_FILES += nrf_ecb.c
C_SOURCE_FILES += nrf_nvmc.c
C_SOURCE_FILES += pstorage.c
#C_SOURCE_FILES += radio_config.c
C_SOURCE_FILES += spi_master.c
C_SOURCE_FILES += twi_hw_master.c
C_SOURCE_FILES += radio_config.c
C_SOURCE_FILES += bootloader.c
C_SOURCE_FILES += bootloader_settings.c
C_SOURCE_FILES += bootloader_util.c
C_SOURCE_FILES += dfu_init_template.c
C_SOURCE_FILES += dfu_single_bank.c
C_SOURCE_FILES += dfu_transport_serial.c
C_SOURCE_FILES += app_button.c
C_SOURCE_FILES += crc16.c
C_SOURCE_FILES += app_fifo.c
C_SOURCE_FILES += app_gpiote.c
C_SOURCE_FILES += hci_mem_pool.c
C_SOURCE_FILES += hci_slip.c
C_SOURCE_FILES += hci_transport.c
C_SOURCE_FILES += mem_manager.c
C_SOURCE_FILES += app_pwm.c
C_SOURCE_FILES += app_scheduler.c
C_SOURCE_FILES += sensorsim.c
C_SOURCE_FILES += sha256.c
C_SOURCE_FILES += app_timer_appsh.c
C_SOURCE_FILES += app_trace.c
C_SOURCE_FILES += retarget.c
C_SOURCE_FILES += app_error.c
C_SOURCE_FILES += app_util_platform.c
C_SOURCE_FILES += nrf_assert.c
C_SOURCE_FILES += nrf51_ic_info.c
C_SOURCE_FILES += app_timer.c
#C_SOURCE_FILES += app_gpiote.c


DEV_PATH := $(shell pwd)
SDK_PATH := $(DEV_PATH)/nrf51_sdk/

OUTPUT_FILENAME := gcc130

DEVICE_VARIANT := xxaa
#DEVICE_VARIANT := xxab

USE_SOFTDEVICE := S130
#USE_SOFTDEVICE := S110

CFLAGS := -DDEBUG_NRF_USER -DBLE_STACK_SUPPORT_REQD -DSOFTDEVICE_PRESENT -DNRF51 -DS130 -DSPI_MASTER_0_ENABLE

# we do not use heap in this app
ASMFLAGS := -D__HEAP_SIZE=0

# keep every function in separate section. This will allow linker to dump unused functions
# also set project specific Defines here
CFLAGS += -ffunction-sections -fomit-frame-pointer

# let linker to dump unused sections
LDFLAGS := -Wl,--gc-sections

INCLUDEPATHS += -I"$(SDK_PATH)ble"
INCLUDEPATHS += -I"$(SDK_PATH)libraries"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/bootloader_dfu"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/bootloader_dfu/ble_transport"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/bootloader_dfu/hci_transport"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/button"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/console"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/crc16"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/fifo"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/gpiote"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/hci/config"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/hci"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/ic_info"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/mem_manager"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/pwm"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/scheduler"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/sha256"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/sensorsim"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/timer"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/trace"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/uart"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/util"
INCLUDEPATHS += -I"$(SDK_PATH)softdevice/s130/headers"
INCLUDEPATHS += -I"$(SDK_PATH)softdevice/s130/hex"
INCLUDEPATHS += -I"$(SDK_PATH)toolchain/arm"
INCLUDEPATHS += -I"$(SDK_PATH)softdevice/common/softdevice_handler"
INCLUDEPATHS += -I"$(SDK_PATH)softdevice/common"
INCLUDEPATHS += -I"$(SDK_PATH)libraries/bootloader_dfu_experimental"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/application/codecs/s130/middleware"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/application/codecs/s130/serializers"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/application/codecs/s130"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/application/codecs/common"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/application/codecs"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/application/hal"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/application"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/common"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/connectivity/codecs/s130/serializers"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/connectivity/codecs/s130/middleware"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/connectivity/codecs/s130"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/connectivity/codecs/common"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/connectivity/codecs"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/common/transport/ser_phy"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/common/transport/ser_phy/config"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/common/transport/debug"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/common/transport"
INCLUDEPATHS += -I"$(SDK_PATH)serialization/common/struct_ser/s130"
INCLUDEPATHS += -I"$(SDK_PATH)drivers"
INCLUDEPATHS += -I"$(SDK_PATH)toolchain/gcc"
INCLUDEPATHS += -I"$(SDK_PATH)toolchain"
INCLUDEPATHS += -I"$(DEV_PATH)"

C_SOURCE_PATHS += $(SDK_PATH)libraries/bootloader_dfu
C_SOURCE_PATHS += $(SDK_PATH)softdevice/common/softdevice_handler
C_SOURCE_PATHS += $(SDK_PATH)drivers
C_SOURCE_PATHS += $(SDK_PATH)libraries/button
C_SOURCE_PATHS += $(SDK_PATH)libraries/console
C_SOURCE_PATHS += $(SDK_PATH)libraries/crc16
C_SOURCE_PATHS += $(SDK_PATH)libraries/fifo
C_SOURCE_PATHS += $(SDK_PATH)libraries/gpiote
C_SOURCE_PATHS += $(SDK_PATH)libraries/hci/config
C_SOURCE_PATHS += $(SDK_PATH)libraries/hci
C_SOURCE_PATHS += $(SDK_PATH)libraries/ic_info
C_SOURCE_PATHS += $(SDK_PATH)libraries/mem_manager
C_SOURCE_PATHS += $(SDK_PATH)libraries/pwm
C_SOURCE_PATHS += $(SDK_PATH)libraries/scheduler
C_SOURCE_PATHS += $(SDK_PATH)libraries/sha256
C_SOURCE_PATHS += $(SDK_PATH)libraries/sensorsim
C_SOURCE_PATHS += $(SDK_PATH)libraries/timer
C_SOURCE_PATHS += $(SDK_PATH)libraries/trace
C_SOURCE_PATHS += $(SDK_PATH)libraries/uart
C_SOURCE_PATHS += $(SDK_PATH)libraries/util

include $(SDK_PATH)toolchain/gcc/Makefile.common
