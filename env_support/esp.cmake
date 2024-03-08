file(GLOB_RECURSE SOURCES 
        ${FPSDK_ROOT_DIR}/board/esp32/*.c )

idf_component_register(
    SRCS
    ${SOURCES}
    INCLUDE_DIRS
    ${FPSDK_ROOT_DIR}
    ${FPSDK_ROOT_DIR}/board/esp32/inc
    ${FPSDK_ROOT_DIR}/../
    REQUIRES
    main)