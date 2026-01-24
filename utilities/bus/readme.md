# 总线通信组件

本目录提供了各种总线通信的实现，包括Modbus、全双工总线和半双工总线等。

## 文件结构

```
bus/
├── example/              # 总线通信示例
│   ├── simple_mobus_485_port.c
│   └── simple_mobus_485_port.h
├── full/                 # 全双工总线实现
│   ├── full_duplex_bus.c
│   └── full_duplex_bus.h
├── half/                 # 半双工总线实现
│   ├── Readme.md
│   ├── half_duplex_bus.c
│   └── half_duplex_bus.h
├── serial_bus.c          # 串行总线公共实现
└── serial_bus.h          # 串行总线公共头文件
```

## 核心组件说明

### 1. serial_bus.c/h

**功能**：提供串行总线的公共实现，为各种总线通信提供基础支持

**主要内容**：
- 串行总线通用API
- 数据缓冲管理
- 错误检测和处理

### 2. 全双工总线 (full/)

**功能**：提供全双工总线通信实现，支持同时发送和接收数据

**主要内容**：
- 全双工总线初始化和配置
- 数据发送和接收函数
- 中断处理机制

**使用示例**：
```c
#include "full_duplex_bus.h"

// 定义全双工总线配置
static full_duplex_bus_config_t bus_config = {
    .baud_rate = 115200,
    .data_bits = 8,
    .stop_bits = 1,
    .parity = FULL_DUPLEX_BUS_PARITY_NONE
};

// 初始化总线
full_duplex_bus_handle_t bus = full_duplex_bus_init(&bus_config);

// 发送数据
uint8_t send_data[] = {0x01, 0x02, 0x03};
full_duplex_bus_send(bus, send_data, sizeof(send_data));

// 接收数据
uint8_t recv_data[10];
size_t recv_len = full_duplex_bus_receive(bus, recv_data, sizeof(recv_data), 100);

// 关闭总线
full_duplex_bus_deinit(bus);
```

### 3. 半双工总线 (half/)

**功能**：提供半双工总线通信实现，支持分时发送和接收数据

**主要内容**：
- 半双工总线初始化和配置
- 数据发送和接收函数
- 总线状态管理

### 4. Modbus示例 (example/)

**功能**：提供Modbus 485通信的示例实现

**主要内容**：
- Modbus RTU协议实现
- 485端口初始化和配置
- 数据读写示例

**使用示例**：
```c
#include "simple_mobus_485_port.h"

// 初始化Modbus 485端口
modbus_485_port_t *modbus_port = modbus_485_port_init(115200);

// 读取保持寄存器
uint16_t reg_value;
if (modbus_485_port_read_holding_registers(modbus_port, 0x01, 0x1000, 1, &reg_value) == MODBUS_OK) {
    printf("Register value: %d\n", reg_value);
}

// 写入保持寄存器
if (modbus_485_port_write_holding_register(modbus_port, 0x01, 0x1000, 1234) == MODBUS_OK) {
    printf("Register written successfully\n");
}

// 关闭Modbus 485端口
modbus_485_port_deinit(modbus_port);
```

## 选择指南

| 总线类型 | 适用场景 | 特点 |
| ------ | ------ | ---- |
| 全双工总线 | 需要同时发送和接收数据的场景 | 通信效率高，实现相对复杂 |
| 半双工总线 | 不需要同时发送和接收数据的场景 | 实现简单，硬件成本低 |
| Modbus 485 | 工业自动化、传感器网络等场景 | 协议成熟，抗干扰能力强 |

## 使用建议

1. 根据通信需求选择合适的总线类型
2. 合理配置总线参数（波特率、数据位、停止位等）
3. 实现适当的错误处理机制
4. 对于长距离通信，考虑使用Modbus 485等抗干扰能力强的协议