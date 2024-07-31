# Makefile for fp-sdk-demo
# Compiler
CC := gcc

# Compiler flags
CFLAGS := -DFP_LOG_TRACE_DATA_CENTER

# Source files directory
DEMO_DIR := demo

# Include directories
UNITY_DIR := ../../Unity/src

# Source files
SRCS := test_data_center.c data_center.c account.c ../pingpong/pingpong.c ../soft_timer/fp_soft_timer.c
SRCS += $(wildcard $(DEMO_DIR)/*.c)
SRCS += $(wildcard $(UNITY_DIR)/*.c)
# Executable name
TARGET := test_data_center

# Default target
all: $(TARGET)

# Rule to make the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -I$(UNITY_DIR) -I$(DEMO_DIR)
# Clean target
clean:

# Phony targets
.PHONY: all clean