set(SUBDIRS
    log
    # clists
    # common
)

# 添加子目录
foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
    target_link_libraries(fp-sdk INTERFACE ${subdir})
endforeach()
