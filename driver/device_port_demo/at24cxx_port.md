依赖于软件i2c组件(i2c_bus)

实现配置config，ops以及bus_name即可使用。详见注释


提供的接口是 device_write以及device_read来读写。参数里面的addition是地址，len为长度。


需要注意的是config配置必须和实际的型号对应，否则无法使用。

目前支持如下的几个配置

```c
at24cxx_describe_t my_at24c32 = {
    .bus_name = "my_i2c",
    .config = {
        .ee_dev_addr = 0x50,
        .ee_addr_btyes = 2,
        .ee_page_size = 32,
        .ee_size = (4 * 1024),
    },
    .ops = {
        .init = NULL,
        .deinit = NULL,
        .lock = NULL,
        .unlock = NULL,
        .vcc_enable = NULL,
        .wp_enable = NULL,
    },
};

at24cxx_describe_t my_at24c64 = {
    .bus_name = "my_i2c",
    .config = {
        .ee_dev_addr = 0x50,
        .ee_addr_btyes = 2,
        .ee_page_size = 32,
        .ee_size = (8 * 1024),
    },
    .ops = {
        .init = NULL,
        .deinit = NULL,
        .lock = NULL,
        .unlock = NULL,
        .vcc_enable = NULL,
        .wp_enable = NULL,
    },
};

at24cxx_describe_t my_at24c512 = {
    .bus_name = "my_i2c",
    .config = {
        .ee_dev_addr = 0x50,
        .ee_addr_btyes = 2,
        .ee_page_size = 64,
        .ee_size = (16 * 1024),
    },
    .ops = {
        .init = NULL,
        .deinit = NULL,
        .lock = NULL,
        .unlock = NULL,
        .vcc_enable = NULL,
        .wp_enable = NULL,
    },
};

at24cxx_describe_t my_at24c256 = {
    .bus_name = "my_i2c",
    .config = {
        .ee_dev_addr = 0x50,
        .ee_addr_btyes = 2,
        .ee_page_size = 64,
        .ee_size = (32 * 1024),
    },
    .ops = {
        .init = NULL,
        .deinit = NULL,
        .lock = NULL,
        .unlock = NULL,
        .vcc_enable = NULL,
        .wp_enable = NULL,
    },
};

at24cxx_describe_t my_at24c512 = {
    .bus_name = "my_i2c",
    .config = {
        .ee_dev_addr = 0x50,
        .ee_addr_btyes = 2,
        .ee_page_size = 128,
        .ee_size = (64 * 1024),
    },
    .ops = {
        .init = NULL,
        .deinit = NULL,
        .lock = NULL,
        .unlock = NULL,
        .vcc_enable = NULL,
        .wp_enable = NULL,
    },
};
```
