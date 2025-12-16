# 设备IO驱动框架核心代码

本目录包含设备IO驱动框架的核心代码，提供了设备和驱动的注册、管理和通用API接口。

## 文件列表

| 文件名 | 描述 |
| ------ | ---- |
| demo.sct | MDK分散加载文件示例，用于定义驱动和设备的内存区域 |
| device.c | 设备管理核心实现，包含设备的打开、关闭、读写等API |
| device.h | 设备相关定义和宏，包括设备结构体和设备注册宏 |
| driver.c | 驱动管理核心实现，包含驱动和设备的匹配函数 |
| driver.h | 驱动相关定义和宏，包括驱动结构体和驱动注册宏 |

## 核心概念

### 驱动(driver)
驱动是对一类设备操作的抽象，提供统一的API接口。使用`DRIVER_DEFINED`宏定义驱动。

### 设备(device)
设备是具体硬件的抽象，通过设备注册将具体硬件与驱动关联。使用`DEVICE_DEFINED`宏定义设备。

## 核心函数说明

### 驱动管理函数

#### `driver_search_device()`
```c
int32_t driver_search_device(void);
```
- **功能**：遍历所有注册的驱动和设备，将设备与对应的驱动进行匹配
- **返回值**：成功返回E_OK，失败返回错误码
- **使用场景**：系统初始化时调用，用于建立设备和驱动的关联

### 设备操作函数

#### `device_open(char *name)`
```c
void *device_open(char *name);
```
- **功能**：打开指定名称的设备
- **参数**：
  - `name`：设备名称
- **返回值**：成功返回设备指针，失败返回NULL
- **使用场景**：在使用设备前调用，初始化设备资源

#### `device_close(device_t *dev)`
```c
void device_close(device_t *dev);
```
- **功能**：关闭指定设备
- **参数**：
  - `dev`：设备指针
- **返回值**：无
- **使用场景**：不再使用设备时调用，释放设备资源

#### `device_write(device_t *dev, void *buf, uint32_t addition, uint32_t len)`
```c
int32_t device_write(device_t *dev, void *buf, uint32_t addition, uint32_t len);
```
- **功能**：向设备写入数据
- **参数**：
  - `dev`：设备指针
  - `buf`：要写入的数据缓冲区
  - `addition`：附加参数（具体含义由驱动定义）
  - `len`：要写入的数据长度
- **返回值**：成功返回E_OK，失败返回错误码
- **使用场景**：向设备发送数据时调用

#### `device_read(device_t *dev, void *buf, uint32_t addition, uint32_t len)`
```c
int32_t device_read(device_t *dev, void *buf, uint32_t addition, uint32_t len);
```
- **功能**：从设备读取数据
- **参数**：
  - `dev`：设备指针
  - `buf`：存储读取数据的缓冲区
  - `addition`：附加参数（具体含义由驱动定义）
  - `len`：要读取的数据长度
- **返回值**：成功返回E_OK，失败返回错误码
- **使用场景**：从设备接收数据时调用

#### `device_ioctl(device_t *dev, uint32_t cmd, void *args)`
```c
int32_t device_ioctl(device_t *dev, uint32_t cmd, void *args);
```
- **功能**：向设备发送控制命令
- **参数**：
  - `dev`：设备指针
  - `cmd`：控制命令（预定义或驱动自定义）
  - `args`：命令参数（具体格式由命令定义）
- **返回值**：成功返回E_OK，失败返回错误码
- **使用场景**：对设备进行特殊控制操作时调用

#### `device_irq_process(device_t *dev, uint32_t irq_handler, void *args, uint32_t len)`
```c
int32_t device_irq_process(device_t *dev, uint32_t irq_handler, void *args, uint32_t len);
```
- **功能**：处理设备中断或轮询事件
- **参数**：
  - `dev`：设备指针
  - `irq_handler`：中断处理类型（具体含义由驱动定义）
  - `args`：事件参数
  - `len`：参数长度
- **返回值**：成功返回E_OK，失败返回错误码
- **使用场景**：在中断服务程序或轮询循环中调用，处理设备事件

## 使用示例

### 定义驱动
```c
// 定义一个名为"my_driver"的驱动
DRIVER_DEFINED(my_driver, _my_open, _my_close, _my_write, _my_read, _my_ioctl, _my_irq_handler);

// 实现驱动函数
static fp_err_t _my_open(driver_t **drv) {
    // 驱动打开逻辑
    return E_OK;
}

// 其他驱动函数实现...
```

### 定义设备
```c
// 定义一个名为"my_device"的设备，关联到"my_driver"驱动
static my_driver_describe_t my_device_desc = {
    // 设备描述信息
};

DEVICE_DEFINED(my_device, my_driver, &my_device_desc);
```

### 使用设备
```c
// 初始化驱动和设备关联
driver_search_device();

// 打开设备
device_t *dev = device_open("my_device");
if (dev) {
    // 向设备写入数据
    uint8_t data[] = {0x01, 0x02, 0x03};
    device_write(dev, data, 0, sizeof(data));
    
    // 从设备读取数据
    uint8_t read_buf[10];
    device_read(dev, read_buf, 0, sizeof(read_buf));
    
    // 发送控制命令
    device_ioctl(dev, IOCTL_MY_DEVICE_CUSTOM_CMD, &cmd_args);
    
    // 关闭设备
    device_close(dev);
}
```

## 分散加载文件配置

使用该驱动框架需要在分散加载文件中定义`DEV_REGION`和`DRV_REGION`两个内存区域，示例配置如下：

```sct
LR_IROM1 0x08000000 0x00020000  {
  ER_IROM1 0x08000000 0x00020000  {
   *.o (RESET, +First)
   *(InRoot$$Sections)
   .ANY (+RO)
   .ANY (+XO)
  }

  DEV_REGION 0x20000000 0x00000200  {
   *(.dev_defined)
  }
  DRV_REGION 0x20000100 0x00000200  {
   *(.drv_defined)
  }
  RW_IRAM1 0x20000200 0x00004C00  {
   .ANY (+RW +ZI)
  }
}
```

每个驱动或设备占用32字节内存，实际使用时可根据需要调整区域大小。