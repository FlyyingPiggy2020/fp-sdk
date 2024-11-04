COMPONENT_ADD_INCLUDEDIRS += TLSF-2.4.6/src
COMPONENT_SRCDIRS += TLSF-2.4.6/src
# 如果要使用export机制需要增加这个编译选项
COMPONENT_ADD_LDFLAGS_HEAD += -Wl,--whole-archive
COMPONENT_ADD_LDFLAGS_TAIL += -Wl,--no-whole-archive