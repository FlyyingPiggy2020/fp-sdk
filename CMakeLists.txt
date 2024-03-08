cmake_minimum_required(VERSION 3.12.4)

set(FPSDK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})


if (ESP_PLATFORM) # ESP-IDF
    
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/esp.cmake)
else()

endif()
# cmake -S ..  -G "Unix Makefiles"