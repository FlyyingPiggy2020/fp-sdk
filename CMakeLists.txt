CMAKE_MINIMUM_REQUIRED(VERSION 3.25) # 设定版本
PROJECT(fp-test) # 设定工程名

# set(CMAKE_CXX_COMPILER "gcc") # 设定编译器
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加接口库
add_library(fp-sdk INTERFACE)

# 添加子目录
add_subdirectory(utilities)
add_subdirectory(components)

# 添加生成目标
add_executable(Demo main.c)

# 添加链接库
target_link_libraries(Demo fp-sdk)

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo "Running Demo..."
    COMMAND Demo.exe || ${CMAKE_COMMAND} -E echo "Demo returned non-zero exit code, but continuing..."
    DEPENDS Demo
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# cmake -S ..  -G "Unix Makefiles"