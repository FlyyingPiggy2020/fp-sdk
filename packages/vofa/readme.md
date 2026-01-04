# VOFA+ 调试输出包

基于JustFloat协议的VOFA+调试输出库，用于实时波形可视化。

## 协议格式

JustFloat协议数据格式：
```
[float0][float1]...[floatN][NaN帧尾]
```

- 每个float占4字节(IEEE 754单精度)
- NaN帧尾为 `0x00,0x00,0x80,0x7F` (小端序)

## 移植步骤

### 1. 初始化

```c
#include "vofa.h"

device_t *uart = device_open("usart3");
vofa_init(uart);
```

### 2. 发送数据

```c
// 发送多通道数据(最多8通道)
float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
vofa_send(data, 4);
```

## VOFA+配置

1. 打开VOFA+软件
2. 选择协议: `JustFloat`
3. 设置波特率: 与UART配置一致(默认115200)
4. 打开串口即可显示波形

## 注意事项

- 最大支持8通道浮点数据
- 非阻塞发送,适合高频调用
- 需要确保UART设备已正确配置
