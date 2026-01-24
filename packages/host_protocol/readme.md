# host_protocol - 通用上位机通讯协议层

## 概述

`host_protocol` 是一个设备无关的通用上位机通讯协议层，实现统一的帧格式编解码功能。

## 设计原则

- **设备无关**: 协议层不包含具体设备的业务逻辑
- **无静态变量**: 支持多实例，无全局静态变量
- **回调驱动**: 通过回调函数处理命令，由应用层控制逻辑
- **简单高效**: 提供帧打包/解析基本功能，不添加复杂特性

## 帧格式

```
+--------+--------+--------+--------+--------+--------+--------+
|  帧头   |  版本   |  类型   |  序列号  |  长度   |  载荷   | CRC16  |
| 0x55AA |  0x00  | 0xXX  | 2字节  | 2字节  |  N字节  | 2字节  |
+--------+--------+--------+--------+--------+--------+--------+
```

| 域 | 长度 | 说明 |
|---|-----|------|
| 帧头 | 2 | 固定值 `0x55AA` (小端) |
| 版本 | 1 | 协议版本，当前为 `0x00` |
| 类型 | 1 | 决定载荷解析方式，最高位为响应标志位 |
| 序列号 | 2 | 用于请求-应答匹配 |
| 长度 | 2 | 载荷长度 (小端) |
| 载荷 | N | 实际数据 |
| CRC16 | 2 | 对整个帧的校验 (多项式0x8005, 初值0xFFFF) |

## 类型字段

| 值 | 方向 | 说明 |
|---|-----|------|
| 0x01 | MCU→上位机 | 波形数据1 |
| 0x02 | 上位机→MCU | 波形数据2 |
| 0x03 | 上位机→MCU | 命令数据 |
| 0x04 | MCU→上位机 | 主动上报 |
| 0x8X | - | 应答标志位 (D7置位) |

## API说明

### 打包数据帧

```c
int host_pack_frame(uint8_t type, uint16_t seq,
                    const uint8_t *payload, uint16_t len,
                    uint8_t *buf, uint16_t buf_size);
```

### 解析接收数据

```c
int host_parse_frame(const uint8_t *data, uint16_t len,
                     struct host_frame *frame);
```

### 处理命令

```c
int host_handle_command(const struct host_frame *frame,
                        host_cmd_handler_t handler, void *userdata);
```

### 生成应答

```c
int host_pack_response(uint8_t cmd_type, uint16_t seq, uint8_t result,
                       uint8_t *buf, uint16_t buf_size);
```

### CRC16计算

```c
uint16_t host_crc16(const uint8_t *data, uint16_t len);
```

## 使用示例

```c
#include "host_protocol.h"

/* 命令处理回调 */
static int my_cmd_handler(uint16_t cmd_id, const uint8_t *param,
                          uint16_t len, void *userdata)
{
    switch (cmd_id) {
        case 0x0001:
            /* 处理命令1 */
            break;
        case 0x0002:
            /* 处理命令2 */
            break;
        default:
            break;
    }
    return 0;
}

/* 发送波形数据 */
void send_wave_data(device_t *uart, const float *data, int count)
{
    uint8_t buf[128];
    uint16_t seq = 0;
    int len;

    len = host_pack_frame(HOST_TYPE_WAVE1, seq,
                          (uint8_t *)data, count * sizeof(float),
                          buf, sizeof(buf));

    if (len > 0) {
        device_write(uart, buf, 0, len);
    }
}

/* 接收并解析数据 */
void receive_data(device_t *uart)
{
    uint8_t rx_buf[256];
    struct host_frame frame;
    int rx_len;

    rx_len = device_read(uart, rx_buf, 0, sizeof(rx_buf));
    if (rx_len <= 0) {
        return;
    }

    if (host_parse_frame(rx_buf, rx_len, &frame) == 0) {
        switch (frame.type) {
            case HOST_TYPE_CMD:
                host_handle_command(&frame, my_cmd_handler, NULL);
                /* 发送应答 */
                uint8_t resp_buf[16];
                int resp_len = host_pack_response(HOST_TYPE_CMD, frame.seq, 0,
                                                  resp_buf, sizeof(resp_buf));
                device_write(uart, resp_buf, 0, resp_len);
                break;
            case HOST_TYPE_WAVE2:
                /* 处理波形数据2 */
                break;
            default:
                break;
        }
    }
}
```

## 设备适配

本协议层不包含具体设备的业务逻辑。设备特定的波形数据格式和命令列表由应用层实现。

参见 `app/app_usb_cdc.c` 中的FBR2101适配层实现。
