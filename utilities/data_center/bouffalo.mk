include $(BL60X_SDK_PATH)/components/network/ble/ble_common.mk
COMPONENT_ADD_INCLUDEDIRS += inc
COMPONENT_SRCDIRS += src
# 如果要使用export机制需要增加这个编译选项
COMPONENT_ADD_LDFLAGS_HEAD += -Wl,--whole-archive
COMPONENT_ADD_LDFLAGS_TAIL += -Wl,--no-whole-archive