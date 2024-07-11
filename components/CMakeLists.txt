# 使用file(GLOB ...)是不安全的，这种遍历寻找的方式虽然很方便，并不被cmake官方推荐。
# https://cmake.org/cmake/help/v3.14/command/file.html?highlight=file#filesystem
# https://stackoverflow.com/questions/32411963/why-is-cmake-file-glob-evil
# 因为会包含你不需要的内容，并且多人合作的时候，添加你不需要的编译的文件进来，所以这里采用手动添加的方式
set(SUBDIRS
    led_flash
)

# 添加子目录
# 把子目录链接到fp-sdk这个接口库，注意这里需要子目录的链接库和子目录名称相同，比如log的链接库名称也要叫log才能被正确添加
foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
    target_link_libraries(fp-sdk INTERFACE ${subdir})
endforeach()
