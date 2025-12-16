# 公共头文件目录

本目录包含fp-sdk的公共头文件，提供了基础定义、接口声明和工具函数。

## 文件列表

| 文件名 | 描述 |
| ------ | ---- |
| board_options_sample.h | 板级配置文件示例 |
| clists.h | 循环链表实现 |
| define.h | 基础定义和宏 |
| errorno.h | 错误码定义 |
| event.h | 事件机制实现 |
| misc.h | 杂项工具函数 |
| options.h | 主配置文件 |

## 头文件详细说明

### 1. board_options_sample.h

**功能**：板级配置文件示例，用户可以基于此创建自己的板级配置文件

**主要内容**：
- 硬件平台相关配置
- 外设引脚定义
- 时钟配置
- 中断优先级配置

**使用说明**：
1. 复制该文件并重命名为`board_options.h`
2. 根据目标硬件平台修改配置
3. 在工程中添加`FP_INCLDUE_BOARD_OPTIONS_FILE`宏定义
4. 包含`options.h`头文件即可自动包含`board_options.h`

### 2. clists.h

**功能**：提供循环链表的实现，用于高效的节点插入、删除和遍历操作

**主要内容**：
- 循环链表节点结构体定义
- 链表初始化、插入、删除、遍历等操作函数

**使用示例**：

略。标准的linux内核的双向链表实现，使用示例网络上很多了。

### 3. define.h

**功能**：提供基础定义和宏，包括数据类型、位操作、编译选项等

**主要内容**：
- 数据类型定义（uint8_t, uint16_t, uint32_t等）
- 位操作宏（置位、清零、翻转等）
- 编译选项宏
- 内存对齐宏

### 4. errorno.h

**功能**：提供错误码定义，用于函数返回值错误判断

**主要内容**：
- 标准错误码（E_OK, E_ERROR, E_BUSY等）
- 自定义错误码

**使用示例**：
```c
#include "errorno.h"

int my_function(void) {
    if (/* 操作失败 */) {
        return E_ERROR;
    }
    if (/* 资源忙 */) {
        return E_BUSY;
    }
    return E_OK;
}

// 调用函数并判断返回值
if (E_OK != my_function()) {
    // 处理错误
}
```

### 5. event.h

**功能**：提供事件机制实现，用于线程间或中断与线程间的通信

**主要内容**：
- 事件结构体定义
- 事件初始化、设置、等待、清除等操作函数

**使用示例**：
```c
#include "event.h"

// 定义事件
static event_t my_event;

// 初始化事件
event_init(&my_event, 0);

// 在一个线程中等待事件
if (event_wait(&my_event, EVENT_FLAG_0, EVENT_WAIT_ANY, WAIT_FOREVER) == EVENT_FLAG_0) {
    // 事件被触发
}

// 在另一个线程或中断中设置事件
event_set(&my_event, EVENT_FLAG_0);
```

### 6. misc.h

**功能**：提供杂项工具函数，包括字符串操作、数学计算、内存操作等

**主要内容**：
- 字符串操作函数
- 数学计算函数
- 内存操作函数
- 其他通用工具函数

### 7. options.h

**功能**：主配置文件，包含SDK的核心配置和接口声明

**主要内容**：
- 内存管理配置
- 系统API定义
- 日志功能配置
- 断言配置

**使用说明**：
- 用户代码只需包含此头文件即可访问SDK的所有功能
- 可通过宏定义配置SDK的行为
- 自动包含其他必要的头文件

## 头文件包含关系

```
options.h
├── define.h
├── misc.h
├── errorno.h
├── event.h
└── clists.h
    └── define.h
```

## 使用建议

1. 始终包含`options.h`而不是直接包含其他头文件，除非有特殊需求
2. 根据需要在`board_options.h`中配置硬件相关参数
3. 遵循SDK的命名规范和代码风格
4. 利用错误码进行错误处理，提高代码健壮性