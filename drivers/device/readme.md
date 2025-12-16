# 设备驱动实现

本目录包含了各种具体设备的驱动实现，这些驱动基于设备IO框架核心代码开发，提供了对不同硬件设备的操作接口。

## 文件列表

| 文件名 | 设备类型 | 描述 |
| ------ | ------ | ---- |
| analog.c/h | 模拟量 | 模拟量输入输出驱动 |
| at24cxx.c/h | EEPROM | AT24CXX系列EEPROM驱动 |
| dev_can.c/h | CAN总线 | CAN总线设备驱动 |
| hall.c/h | 霍尔传感器 | 霍尔传感器驱动 |
| i2c_bus.c/h | I2C总线 | I2C总线驱动 |
| led_flash.c/h | LED闪光灯 | LED闪光灯驱动 |
| lightc_map.c/h | 调光器 | 调光器驱动，支持多种调光曲线 |
| paj7620.c/h | 手势识别 | PAJ7620手势识别传感器驱动 |
| pwmc.c/h | PWM控制器 | PWM控制器驱动 |
| roller_blind_control.c/h | 卷帘控制 | 卷帘电机控制驱动 |

## 驱动通用使用方法

每个设备驱动都遵循以下通用使用模式：

1. **定义设备描述结构体**：根据驱动头文件中定义的描述结构体，填充具体硬件的操作函数和参数
2. **注册设备**：使用`DEVICE_DEFINED`宏将设备注册到对应的驱动
3. **初始化驱动和设备关联**：调用`driver_search_device()`函数
4. **使用设备**：通过设备IO框架提供的通用API（`device_open`, `device_close`, `device_ioctl`等）操作设备

## 各设备驱动详细说明

### 1. 模拟量驱动 (analog.c/h)

**功能**：提供模拟量输入输出功能

**核心API**：
- `IOCTL_ANALOG_ENABLE`：启用模拟量功能
- `IOCTL_ANALOG_DISABLE`：禁用模拟量功能
- `IOCTL_ANALOG_GET`：获取模拟量值
- `IOCTL_ANALOG_SET_IRQ_HANDLER`：设置中断处理函数

**使用示例**：
```c
// 定义模拟量设备描述
static analog_describe_t analog_desc = {
    .number_of_channels = 2,
    .ops = {
        .init = analog_init,
        .deinit = analog_deinit,
        .enable = analog_enable,
        .get = analog_get,
        .irq_handler = analog_irq_handler
    }
};

// 注册设备
DEVICE_DEFINED(analog_dev, analog, &analog_desc);

// 使用设备
device_t *dev = device_open("analog_dev");
if (dev) {
    // 获取通道0的模拟量值
    union analog_ioctl_param param;
    param.get.channel = 0;
    device_ioctl(dev, IOCTL_ANALOG_GET, &param);
    printf("Analog value: %d\n", param.get.data);
    
    device_close(dev);
}
```

### 2. EEPROM驱动 (at24cxx.c/h)

**功能**：提供AT24CXX系列EEPROM的读写功能

**使用说明**：
- 支持不同容量的AT24CXX芯片
- 提供字节读写和页读写功能
- 支持I2C总线接口

### 3. CAN总线驱动 (dev_can.c/h)

**功能**：提供CAN总线通信功能

**使用说明**：
- 支持标准帧和扩展帧
- 提供发送和接收功能
- 支持中断和轮询模式

### 4. 霍尔传感器驱动 (hall.c/h)

**功能**：提供霍尔传感器的检测功能

**使用说明**：
- 支持磁场强度检测
- 提供中断和轮询模式
- 可用于电机测速、位置检测等应用

### 5. I2C总线驱动 (i2c_bus.c/h)

**功能**：提供I2C总线通信功能

**使用说明**：
- 支持主模式和从模式
- 提供字节读写和批量读写功能
- 支持多种速率配置

### 6. LED闪光灯驱动 (led_flash.c/h)

**功能**：提供LED闪光灯的控制功能

**使用说明**：
- 支持多种闪光模式
- 可配置闪光频率和占空比
- 支持紧急闪光功能

### 7. 调光器驱动 (lightc_map.c/h)

**功能**：提供LED调光功能，支持多种调光曲线

**使用说明**：
- 支持线性调光、对数调光等多种曲线
- 可配置调光范围和精度
- 支持PWM调光方式

### 8. 手势识别驱动 (paj7620.c/h)

**功能**：提供PAJ7620手势识别传感器的控制功能

**使用说明**：
- 支持多种手势识别（上下左右、前后、顺时针逆时针等）
- 提供中断和轮询模式
- 可配置灵敏度

### 9. PWM控制器驱动 (pwmc.c/h)

**功能**：提供PWM信号生成功能

**使用说明**：
- 支持多个PWM通道
- 可配置频率和占空比
- 支持硬件PWM和软件PWM

### 10. 卷帘控制驱动 (roller_blind_control.c/h)

**功能**：提供卷帘电机的控制功能

**使用说明**：
- 支持上下限位检测
- 可配置运行速度和力度
- 支持手动和自动模式

## 注意事项

1. 每个设备驱动都有自己的描述结构体，需要根据具体硬件实现对应的操作函数
2. 设备名称和驱动名称必须唯一
3. 使用前确保已经正确初始化了底层硬件
4. 对于需要中断的设备，确保已经正确配置了中断向量和优先级