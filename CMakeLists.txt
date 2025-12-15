cmake_minimum_required(VERSION 3.22)

#fp-sdk
set(FP_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INC_VPATH ${FP_SDK_DIR}/inc ${FP_SDK_DIR}/driver/core ${FP_SDK_DIR}/driver/device)
set(CFILES ${FP_SDK_DIR}/driver/core/device.c ${FP_SDK_DIR}/driver/core/driver.c)

# device
if(DEFINED ANALOG)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/analog.c)
endif()

if(DEFINED AT24CXX)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/at24cxx.c)
endif()

if(DEFINED HALL)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/hall.c)
endif()

if(DEFINED I2CBUS)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/i2c_bus.c)
endif()

if(DEFINED LED_FLASH)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/led_flash.c)
endif()

if(DEFINED LIGHTC)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/lightc_map.c)
endif()

if(DEFINED PAJ7620)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/paj7620.c)
endif()

if(DEFINED PWMC)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/pwmc.c)
endif()

if(DEFINED ROLLER_BLINDC)
    list(APPEND CFILES ${FP_SDK_DIR}/driver/device/roller_blind_control.c)
endif()

#utility
if(DEFINED BUS)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/bus/serial_bus.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/bus)
endif()

if(DEFINED BUTTER)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/butter/butter.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/butter)
endif()

if(DEFINED DATACENTER)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/data_center/account.c)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/data_center/data_center.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/data_center)
endif()

if(DEFINED EXPORT)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/export/export.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/export)
endif()

if(DEFINED FPLOG)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/fplog/fplog.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/fplog)
endif()

if(DEFINED HEAP4)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/heap4/heap.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/heap4)
endif()

if(DEFINED PID)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/pid/pid.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/pid)
endif()

if(DEFINED PINGPONG)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/pingpong/pingpong.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/pingpong)
endif()

# if(DEFINED SHELL)
# endif()

if(DEFINED SOFT_TIMER)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/soft_timer/soft_timer.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/soft_timer)
endif()

if(DEFINED STORAGE)
    list(APPEND CFILES ${FP_SDK_DIR}/utilities/storage/storage.c)
    list(APPEND INC_VPATH ${FP_SDK_DIR}/utilities/storage)
endif()

target_sources(${PROGRAM_NAME} PRIVATE ${CFILES})
target_include_directories(${PROGRAM_NAME} PRIVATE ${INC_VPATH})
# cmake -S ..  -G "Unix Makefiles"