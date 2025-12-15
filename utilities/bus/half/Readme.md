# 半双工总线驱动程序

## 使用说明

1. 实现 `half_duplex_bus_ops_t`内的函数。

   ```
   typedef struct half_duplex_bus_ops {
       /**
        * @brief 发送数据
        * @param {unsigned char} *buf 待发送数据指针
        * @param {unsigned short} len 待发送数据长度
        * @return {*} 实际发送的数据长度
        */
       int (*write_buf)(unsigned char *buf, unsigned short len);
       /**
        * @brief 接收数据
        * @param {unsigned char} *buf 待接收的数据指针
        * @param {unsigned short} len 希望接收的数据长度
        * @return {*} 实际接收的数据长度
        */
       int (*read_buf)(unsigned char *buf, unsigned short len);
       /**
        * @brief 是否允许发送，用于实现总线防冲突。用户自定义实现。
        * @param {void}
        * @return {*}
        */
       bool (*is_allowed_sending)(void);
       /**
        * @brief 是否允许接收，用于实现总线接收分帧超时。用户自定义实现。
        * @return {*}
        */
       bool (*is_allowed_phase)(void);
       /**
        * @brief 发送延时，用于半双工发送数据帧之间延时。用户自定义实现。
        * @param {unsigned short} ms 延时时间单位毫秒
        * @return {*}
        */
       void (*send_delay)(unsigned short ms);
       /**
        * @brief 数据解析，里面附带了当前发送的数据，可以实现重发停止的功能
        * @param {unsigned char} *recvbuf 接收的数据指针
        * @param {unsigned short} recvlen 接收的数据长度
        * @param {unsigned char} transbuf 正在发送的数据指针
        * @param {unsigned short} translen 正在发送的数据长度
        * @return {*}
        */
       void (*phase)(unsigned char *recvbuf, unsigned short recvlen, unsigned char *transbuf, unsigned short translen);
       /**
        * @brief 中断关闭，如果在中断里调用需要开关中断，里面有原子操作不允许被打断。
        * @return {*}
        */
       void (*lock)(void);
       /**
        * @brief 中断打开，如果在中断里调用需要开关中断，里面有原子操作不允许被打断。
        * @return {*}
        */
       void (*unlock)(void);
   } half_duplex_bus_ops_t;
   ```
2. 调用 `half_duplex_bus_new`申请一条半双工总线。

## API说明

### 1.half_duplex_bus_new

```c
static half_duplex_bus_t *protocol_bus = NULL;
half_duplex_bus_ops_t ops = {
    .is_allowed_phase = _is_allowed_phase,
    .is_allowed_sending = _is_allowed_sending,
    .lock = _lock,
    .phase = _phase,
    .send_delay = _send_delay,
    .unlock = _unlock,
    .write_buf = _write_buf,
    .read_buf = _read_buf,
};

protocol_bus = half_duplex_bus_new(&ops, 256);
```

### 2.half_duplex_bus_transmitter_cache

```c
half_duplex_bus_transmitter_cache(protocol_bus, buf, len, retrans, priority);
```

`retrans`重发次数

`priority`优先级

## 设计说明

### 1.为什么不做接收缓存

因为是半双工总线，并且需要做重发机制（收到应答包后停止重发）

如果异步接收的话，发送和接收就无法同步了。（不确定现在这包接收的内容是否和你的发送包对应）

所以不做缓存。
