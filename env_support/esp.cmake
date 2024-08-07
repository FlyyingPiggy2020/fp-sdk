file(GLOB_RECURSE SOURCES 
        ${FPSDK_ROOT_DIR}/board/esp32/*.c
        ${FPSDK_ROOT_DIR}/device/core/*.c
        ${FPSDK_ROOT_DIR}/driver/pin/pin.c
        ${FPSDK_ROOT_DIR}/driver/pin/drv_esp32_pin.c
        ${FPSDK_ROOT_DIR}/driver/serial/*.c

        ${FPSDK_ROOT_DIR}/utilities/export/export.c
        ${FPSDK_ROOT_DIR}/utilities/log/*.c
        ${FPSDK_ROOT_DIR}/utilities/shell/*.c
        ${FPSDK_ROOT_DIR}/utilities/shell/cmds/shell_cmd_clear.c
        )

idf_component_register(
    SRCS
    ${SOURCES}
    INCLUDE_DIRS
    ${FPSDK_ROOT_DIR}
    ${FPSDK_ROOT_DIR}/board/esp32/inc
    ${FPSDK_ROOT_DIR}/device/core/inc

    ${FPSDK_ROOT_DIR}/driver/pin/inc
    ${FPSDK_ROOT_DIR}/driver/serial/inc

    ${FPSDK_ROOT_DIR}/utilities/log/inc
    ${FPSDK_ROOT_DIR}/utilities/export/inc
    ${FPSDK_ROOT_DIR}/utilities/shell/inc
    ${FPSDK_ROOT_DIR}/../
    REQUIRES
    main
    driver

    LDFRAGMENTS
    "utilities/export/export.lf"
    "utilities/shell/shell.lf"
    )
#定义一个宏 USE_ESP ，这样子c代码里面就可以知道外面是esp32在编译
target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DUSE_ESP=1")

target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp32_pin_link_hook")
target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp32_serial_link_hook")
target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp32_cmd_clear_link_hook")