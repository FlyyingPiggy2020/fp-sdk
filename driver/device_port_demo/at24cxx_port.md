依赖于软件i2c组件(i2c_bus)

实现配置config，ops以及bus_name即可使用。详见注释


提供的接口是 device_write以及device_read来读写。参数里面的addition是地址，len为长度。


需要注意的是config配置必须和实际的型号对应，否则无法使用。

目前支持如下的几个配置

```
// #ifdef AT24C32
// #define EE_MODEL_NAME "AT24C32"
// #define EE_DEV_ADDR   0xA0       /* 设备地址 */
// #define EE_PAGE_SIZE  32         /* 页面大小(字节) */
// #define EE_SIZE       (4 * 1024) /* 总容量(字节) */
// #define EE_ADDR_BYTES 2          /* 地址字节个数 */
// #endif

// #ifdef AT24C64
// #define EE_MODEL_NAME "AT24C64"
// #define EE_DEV_ADDR   0xA0       /* 设备地址 */
// #define EE_PAGE_SIZE  32         /* 页面大小(字节) */
// #define EE_SIZE       (8 * 1024) /* 总容量(字节) */
// #define EE_ADDR_BYTES 2          /* 地址字节个数 */
// #endif

// #ifdef AT24C128
// #define EE_MODEL_NAME "AT24C128"
// #define EE_DEV_ADDR   0xA0        /* 设备地址 */
// #define EE_PAGE_SIZE  64          /* 页面大小(字节) */
// #define EE_SIZE       (16 * 1024) /* 总容量(字节) */
// #define EE_ADDR_BYTES 2           /* 地址字节个数 */
// #endif

// #ifdef AT24C256
// #define EE_MODEL_NAME "AT24C256"
// #define EE_DEV_ADDR   0xA0        /* 设备地址 */
// #define EE_PAGE_SIZE  64          /* 页面大小(字节) */
// #define EE_SIZE       (32 * 1024) /* 总容量(字节) */
// #define EE_ADDR_BYTES 2           /* 地址字节个数 */
// #endif

// #ifdef AT24C512
// #define EE_MODEL_NAME "AT24C512"
// #define EE_DEV_ADDR   0xA0        /* 设备地址 */
// #define EE_PAGE_SIZE  128         /* 页面大小(字节) */
// #define EE_SIZE       (64 * 1024) /* 总容量(字节) */
// #define EE_ADDR_BYTES 2           /* 地址字节个数 */
// #endif
```
