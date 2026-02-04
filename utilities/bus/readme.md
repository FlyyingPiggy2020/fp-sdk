# 串行总线组件

本目录提供统一的串行总线通信实现，通过 `write_buf` 的不同实现支持全双工和半双工模式。

## 文件结构

```
bus/
├── readme.md           # 本文件
├── serial_bus.c        # 串行总线统一实现
└── serial_bus.h        # 串行总线统一接口
```

## 设计理念

**"框架层只管'发', 驱动层管'能不能发'"**

框架层负责：
- 优先级队列管理
- 重发机制
- ACK停止重发
- 总线忙避让（write_buf返回<0时不推进状态）

驱动层负责：
- 硬件寄存器操作
- 总线忙检测
- 方向控制（RS485 DE/RE）
- 帧间静默期检测

## 支持的模式

### 全双工模式

适用于：UART、SPI等硬件支持全双工的接口

**特点**：
- 收发独立，可同时进行
- write_buf 直接发送数据

**示例**：
```c
int uart_write(uint8_t *buf, uint16_t len) {
    return UART_Send(buf, len);  // 直接发送
}

struct serial_bus_ops ops = {
    .write_buf = uart_write,
};
serial_bus_t *bus = serial_bus_new(&ops, &cb, 10);
```

### 半双工模式

适用于：RS485、单线UART等需要方向控制的接口

**特点**：
- write_buf 内部处理：
  - RS485方向控制（DE/RE引脚）
  - 帧间静默期检测（Inter-frame Gap）
  - 总线仲裁（与接收字节的时间戳比较）
- 返回 <0 表示总线忙，框架会重试

**示例**：
```c
static uint32_t last_rx_tick = 0;

void uart_rx_isr(uint8_t data) {
    last_rx_tick = get_tick();
    serial_bus_receive(g_bus, &data, 1);
}

int rs485_write(uint8_t *buf, uint16_t len) {
    // 1. 检查帧间静默期 (3.5字节时间)
    if (get_tick() - last_rx_tick < FRAME_GAP_MS) {
        return -EBUSY;  // 总线忙
    }
    // 2. 方向控制
    RS485_DE_TX();
    // 3. 发送
    UART_Send(buf, len);
    // 4. 发送完成后在 TC 中断中记录 last_rx_tick
    return 0;
}

struct serial_bus_ops ops = {
    .write_buf = rs485_write,
};
serial_bus_t *bus = serial_bus_new(&ops, &cb, 10);
```

## API 说明

### serial_bus_ops

```c
struct serial_bus_ops {
    int (*write_buf)(uint8_t *buf, uint16_t len);  // 必需
    void (*send_delay)(uint16_t ms);               // 可选
    void (*lock)(void);                            // 可选
    void (*unlock)(void);                          // 可选
};
```

### serial_bus_cb

```c
struct serial_bus_cb {
    int (*recv_callback)(uint8_t *recv, uint16_t recv_len,
                        uint8_t *trans, uint16_t trans_len);
    uint16_t (*get_delay_ms)(uint8_t trans_cnt, uint8_t trans_max);
};
```

### 优先级定义

```c
enum {
    SERIAL_BUS_PRIORITY_IDLE = 0,
    SERIAL_BUS_PRIORITY_NORMAL,
    SERIAL_BUS_PRIORITY_REGISTER,
    SERIAL_BUS_PRIORITY_ACK,
    SERIAL_BUS_PRIORITY_MAX
};
```

## 使用流程

1. **定义 write_buf 函数**（必需）
2. **定义 serial_bus_ops 结构**
3. **定义 serial_bus_cb 回调结构**（可选）
4. **调用 serial_bus_new 创建总线**
5. **在中断中调用 serial_bus_receive 接收数据**
6. **在主循环中调用 serial_bus_poll 处理发送**
