# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(. DIR_LIB_SRCS)

# 生成链接库
add_library(clists ${DIR_LIB_SRCS})

# 添加include
target_include_directories(clists PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
