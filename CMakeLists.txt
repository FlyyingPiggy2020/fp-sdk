CMAKE_MINIMUM_REQUIRED(VERSION 3.6) # 设定版本
PROJECT(fp-test) # 设定工程名
set(CMAKE_CXX_COMPILER "gcc") # 设定编译器
add_definitions("-lwsock32")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("${PROJECT_SOURCE_DIR}/fp-sdk/utilities/log/inc")

# 添加子目录
add_subdirectory(fp-sdk)

# add_subdirectory(device_config)

# 添加生成目标
add_executable(Demo main.c)

# # 添加链接库
target_link_libraries(Demo fp-sdk)

# target_link_libraries(Demo device_config)

# cmake -S ..  -G "Unix Makefiles"