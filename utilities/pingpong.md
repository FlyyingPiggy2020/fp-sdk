# 乒乓缓冲区组件

本目录提供了一个乒乓缓冲区（Ping-Pong Buffer）的实现，用于高效的数据读写同步和传输。

## 文件结构

```
utilities/
├── pingpong.c    # 乒乓缓冲区实现
├── pingpong.h    # 乒乓缓冲区头文件
└── pingpong.md   # 本说明文件
```

## 核心功能

- 双缓冲区机制，支持读写并行操作
- 简单高效的同步管理
- 支持任意类型的数据存储
- 线程安全的缓冲区访问
- 低延迟的数据交换

## 核心组件说明

### 1. pingpong.h

**功能**：乒乓缓冲区的头文件，包含所有公共API声明

**主要内容**：
- 乒乓缓冲区结构体定义
- 缓冲区初始化和操作函数声明

### 2. pingpong.c

**功能**：乒乓缓冲区的实现文件，包含所有算法实现

**主要内容**：
- 缓冲区初始化
- 读写缓冲区获取
- 读写完成通知

## 基本使用方法

### 1. 定义缓冲区

```c
#include "pingpong.h"

// 定义乒乓缓冲区结构体
static pingpong_buffer_t pingpong;

// 定义两个实际数据缓冲区
#define BUFFER_SIZE 256
static uint8_t buffer0[BUFFER_SIZE];
static uint8_t buffer1[BUFFER_SIZE];
```

### 2. 初始化乒乓缓冲区

```c
// 初始化乒乓缓冲区，将两个实际缓冲区关联起来
pingpong_buffer_init(&pingpong, buffer0, buffer1);
```

### 3. 写数据到缓冲区

```c
// 获取可写缓冲区
void *write_buf;
pingpong_buffer_get_write_buf(&pingpong, &write_buf);

// 向缓冲区写入数据
memcpy(write_buf, source_data, data_length);

// 通知写入完成
pingpong_buffer_set_write_done(&pingpong);
```

### 4. 从缓冲区读数据

```c
// 尝试获取可读缓冲区
void *read_buf;
if (pingpong_buffer_get_read_buf(&pingpong, &read_buf)) {
    // 从缓冲区读取数据
    process_data(read_buf, data_length);
    
    // 通知读取完成
    pingpong_buffer_set_read_done(&pingpong);
}
```

## 乒乓缓冲区结构体说明

```c
typedef struct pingpong_buffer {
    void *buffer[2];             // 两个实际数据缓冲区的指针
    volatile uint8_t write_index;// 当前写入索引
    volatile uint8_t read_index; // 当前读取索引
    volatile uint8_t read_avaliable[2]; // 缓冲区可用性标志
} pingpong_buffer_t;
```

## 核心API函数说明

### 1. pingpong_buffer_init

```c
void pingpong_buffer_init(struct pingpong_buffer *handler, void *buf0, void *buf1);
```

**功能**：初始化乒乓缓冲区

**参数**：
- `handler`：乒乓缓冲区结构体指针
- `buf0`：第一个实际数据缓冲区指针
- `buf1`：第二个实际数据缓冲区指针

**返回值**：无

### 2. pingpong_buffer_get_read_buf

```c
bool pingpong_buffer_get_read_buf(struct pingpong_buffer *handler, void **pread_buf);
```

**功能**：获取一个可读的缓冲区

**参数**：
- `handler`：乒乓缓冲区结构体指针
- `pread_buf`：输出参数，指向可读缓冲区的指针

**返回值**：
- `true`：成功获取到可读缓冲区
- `false`：没有可用的可读缓冲区

### 3. pingpong_buffer_set_read_done

```c
void pingpong_buffer_set_read_done(struct pingpong_buffer *handler);
```

**功能**：通知缓冲区读取完成

**参数**：
- `handler`：乒乓缓冲区结构体指针

**返回值**：无

### 4. pingpong_buffer_get_write_buf

```c
void pingpong_buffer_get_write_buf(struct pingpong_buffer *handler, void **pwrite_buf);
```

**功能**：获取一个可写的缓冲区

**参数**：
- `handler`：乒乓缓冲区结构体指针
- `pwrite_buf`：输出参数，指向可写缓冲区的指针

**返回值**：无

### 5. pingpong_buffer_set_write_done

```c
void pingpong_buffer_set_write_done(struct pingpong_buffer *handler);
```

**功能**：通知缓冲区写入完成

**参数**：
- `handler`：乒乓缓冲区结构体指针

**返回值**：无

## 应用场景

- 高速数据采集和处理
- 实时信号处理
- DMA传输和CPU处理的同步
- 多线程或中断与主程序间的数据交换
- 音频/视频数据处理

## 性能特点

- **低延迟**：无需等待读写操作完成即可继续使用另一个缓冲区
- **高效**：简单的状态标志管理，无需复杂的同步机制
- **线程安全**：使用volatile关键字确保多线程环境下的正确性
- **灵活**：支持任意类型和大小的数据缓冲区

## 使用建议

1. 确保两个实际缓冲区大小相同，以避免数据丢失
2. 在高频率读写场景下，确保读写操作的时间平衡
3. 对于大块数据传输，乒乓缓冲区可以显著提高性能
4. 在中断处理中使用时，注意保持操作的原子性
5. 及时调用`set_read_done`和`set_write_done`以避免缓冲区死锁