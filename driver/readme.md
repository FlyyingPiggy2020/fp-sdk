| 文件夹 | `备注`         |
| ------ | -------------- |
| core   | 核心代码       |
| device | 支持的设备驱动 |

该组件的使用需要修改分散加载文件，core提供了`mdk`的`demo.sct`文件。

# 简单介绍

一个通用的IO框架，设计有"设备驱动"和"具体设备"两个概念。使用的时候除了添加core和device的代码之外，还需要**把具体的设备注册到驱动**上。**“把具体的设备注册到驱动”**这个操作需要用户自己在额外的代码里去实现与这个库无关。如此实现了驱动和具体设备的解耦合，该驱动库里面代码不做修改，修改用户的注册部分代码，就可以让驱动对应不同的硬件IO口。



通用的API接口为`device_open`，`device_close`， `device_write`，`device_read`，`device_ioctl`，`device_irq_process`。

一般设备都抽象成了开关读写四个操作。如果有该设备还有其他特殊的操作则用`ioctol`来调用，具体的使用方式要看各个驱动它独立的文档。

`device_irq_process`这个API可以实现一些中断或者轮询的操作。(例如这个驱动有部分代码需要轮询处理，或者中断处理，就可以把这部分逻辑放在这个里面)。



下面用一个具体的例子来说如何移植和使用`lightc_map.c`这个调光驱动。从而展示**如何把具体的设备注册到驱动**这个操作。

在下面这个例子里，我们先不关注**如何建立自己的新的驱动**这个事情，先从如何使用了解起。

# 移植说明

假设我们想使用`lightc_map.c`这个调光驱动。不要去关心`lightc_map.c`里面的具体实现。不需要修改这个.c文件。

我们只用关心下面这个宏定义，它告诉了我们`lightc_map`是这个驱动的名字。这个名字在注册的时候会被用到。

```c
DRIVER_DEFINED(lightc_map, _light_open, _light_close, NULL, NULL, _light_ioctl, _light_irq_handler);
```



1.首先根据`core/demo.sct`修改分散加载文件，需要`dev_defined`和`drv_defined`这两个`section`。这两个section的size根据你实际的来使用，一个"设备驱动"或"具体设备"需要32字节。下面是一个示例。

```sct
LR_IROM1 0x08000000 0x00020000  {    ; load region size_region
  ER_IROM1 0x08000000 0x00020000  {  ; load address = execution address
   *.o (RESET, +First)
   *(InRoot$$Sections)
   .ANY (+RO)
   .ANY (+XO)
  }

  DEV_REGION 0x20000000 0x00000200  {  ; RW data
   *(.dev_defined)
  }
  DRV_REGION 0x20000100 0x00000200  {  ; RW data
   *(.drv_defined)
  }
  RW_IRAM1 0x20000200 0x00004C00  {  ; RW data
   .ANY (+RW +ZI)
  }
}
```

2.添加`device.c`、`driver.c`、`lightc_map.c`的代码。

3.在main函数里面调用`driver_search_device`。

```c
int main_app(int argc, void *argv[])
{

    driver_search_device();
    while (1) {
    }
}
```

4.用`DEVICE_DEFINED`把具体的设备注册到驱动。

```
DEVICE_DEFINED(light, lightc_map, &ops);
```

这个宏定义就是你注册一个叫“light”的设备到`lightc_map`这个驱动里面。(`lightc_map就是之前提到的DRIVER_DEFINED创建的`) ops怎么来的呢？这个要看具体的驱动的readme文件。

5.完成上述这些步骤之后，就可以调用了这些API了。

```c
static device_t *dev_light = NULL;
dev_light = device_open("light");
device_ioctl(dev_light, IOCTL_LIGHTC_CMD_OFF, NULL);
```

## 如何新建自己的新的驱动

用`DRIVER_DEFINED`这个宏定义把`name, open, close, write,read,ioctl,irq_handler`填上去。不支持的用NULL表示。

每个驱动都需要一个describe结构体，例如`lightc_map_describe_t`。

具体可以阅读现有的代码。
