COMPONENT_ADD_INCLUDEDIRS += inc
COMPONENT_SRCDIRS += 
# 如果要使用export机制需要增加这个编译选项
COMPONENT_ADD_LDFLAGS_HEAD += -Wl,--whole-archive
COMPONENT_ADD_LDFLAGS_TAIL += -Wl,--no-whole-archive