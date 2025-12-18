基于 v5.1 架构（Rich Model + 间隙轮询），调用变得非常简洁。驱动内部接管了复杂的时序，您只需要关注**业务逻辑**。

这里提供一份完整的应用层调用示例代码 `app_steadywin_demo.c`。

### 1. 移植适配 (必要步骤)

首先，在你的项目中实现时间戳获取函数（通常在 `main.c` 或 `board.c` 中）：

```c
// 必须实现此函数，驱动依赖它进行超时计算
uint64_t sw_get_tick_ms(void) {
    // 示例：STM32 HAL库
    return (uint64_t)HAL_GetTick(); 
    // 或者 RT-Thread: return rt_tick_get_millisecond();
}
```

### 2. 完整应用示例

这个示例展示了如何初始化、如何处理回调、以及如何读取自动更新的“丰富数据模型”。

```c
/*
 * File: app_steadywin_demo.c
 */
#include "steadywin_motor.h"
#include <stdio.h>

/* --- 变量定义 --- */
static sw_motor_t m1;          // 电机对象 (包含所有数据和逻辑)
static device_t *can_dev;      // CAN 设备句柄

/* --- 回调函数 (处理事件通知) --- */
void motor_event_callback(sw_motor_t *motor, sw_event_t evt, uint32_t payload)
{
    switch (evt) {
        // [1] 初始化序列完成
        case SW_EVT_INIT_DONE:
            printf("[Motor %d] Init Done! Ver:%u, Rated:%.1fA\n", 
                   motor->motor_id, 
                   motor->cache.fw_version,      // 自动读取的静态数据
                   motor->cache.conf_rated_curr);
            break;

        // [2] 数据更新 (控制反馈 或 监控轮询)
        case SW_EVT_UPDATE:
            // 这里可以做一些高频数据记录，或者什么都不做，主循环直接读 cache
            // printf("Temp: %.1f, Volt: %.1f\n", motor->cache.fb_temp, motor->cache.v_bus);
            break;

        // [3] 错误处理
        case SW_EVT_ERROR:
            printf("[Motor %d] Error/Fault: 0x%02X\n", motor->motor_id, payload);
            break;

        // [4] 超时
        case SW_EVT_TIMEOUT:
            printf("[Motor %d] Timeout! Cmd: 0x%02X\n", motor->motor_id, payload);
            break;
    }
}

/* --- 系统初始化 --- */
void app_motor_init(void)
{
    // 1. 打开 CAN 设备
    can_dev = device_open("can1");

    // 2. 初始化电机 (ID=1, Kt=0.1, Gear=10.0)
    // 此时状态自动设为 SW_STATE_INITIALIZING
    sw_motor_init(&m1, can_dev, 1, 0.1f, 10.0f);

    // 3. 设置回调
    sw_motor_set_cb(&m1, motor_event_callback, NULL);
    
    // 4. 发送启动指令 (非阻塞)
    // 注意：即便发了 Start，内部也会优先执行完 Init 序列(读版本/参数)后再处理 Start
    sw_motor_stop(&m1); // 先确保停止，安全起见
}

/* --- 主任务/主循环 --- */
void app_motor_task(void)
{
    // ---------------------------------------------------------
    // 1. [驱动层] 总线轮询 (必须高频调用)
    // ---------------------------------------------------------
    // 负责把硬件FIFO里的数据读出来，分发给 m1 (以及其他电机)
    sw_bus_poll(can_dev, 32); 

    // ---------------------------------------------------------
    // 2. [中间件层] 逻辑处理 (必须周期调用)
    // ---------------------------------------------------------
    // 负责推进初始化序列、或者在空闲时插入监控指令
    sw_motor_process(&m1);

    // ---------------------------------------------------------
    // 3. [业务层] 你的控制逻辑
    // ---------------------------------------------------------
    
    // 只有初始化完成后才跑业务
    if (m1.logic_state == SW_STATE_RUNNING) {
        
        static uint64_t last_log = 0;
        static uint64_t last_ctrl = 0;
        uint64_t now = sw_get_tick_ms();

        // [A] 模拟控制逻辑 (例如 100Hz 下发指令)
        if (now - last_ctrl >= 10) {
            last_ctrl = now;
            
            // 下发速度控制 (非阻塞)
            // 调用此函数会刷新 last_active_time，暂时抑制监控轮询
            sw_motor_set_speed(&m1, 200.0f); 
        }

        // [B] 打印数据 (直接读结构体，这就是 Rich Model 的好处)
        if (now - last_log >= 500) {
            last_log = now;
            
            printf("--- Status ---\n");
            // 动态数据 (来自 0x94 反馈)
            printf("Speed: %.2f rad/s, Torque: %.2f Nm\n", m1.cache.fb_spd, m1.cache.fb_tor);
            // 监控数据 (来自 间隙轮询 0xB4)
            printf("Vbus:  %.2f V,     Power:  %.2f W\n", m1.cache.v_bus, m1.cache.power);
            // 静态数据 (来自 初始化 0x84)
            printf("MaxSpd:%.1f\n", m1.cache.conf_max_speed);
        }
    }
}
```

### 3. "间隙轮询" 是如何工作的？

当你运行上述代码时，底层的行为如下：

1.  **0ms - 200ms (初始化阶段)**:
    *   `sw_motor_process` 发现状态是 `INITIALIZING`。
    *   它会依次发送 `0xB1`(版本), `0x84`(配置), `0xA2`(参数)。
    *   此时 `app_motor_task` 里的控制逻辑还不会执行。

2.  **200ms+ (运行阶段 - 只有控制)**:
    *   初始化完成，`logic_state` 变为 `RUNNING`。
    *   你的业务逻辑每 **10ms** 调用一次 `sw_motor_set_speed`。
    *   `set_speed` 会把状态设为 `BUSY`。
    *   收到电机反馈后，`fb_spd` 更新，状态变回 `IDLE`。

3.  **运行阶段 - 间隙监控 (自动触发)**:
    *   假设 CAN 通信很快，发收一次耗时 1ms。
    *   那么在 10ms 的控制周期里，前 1ms 是忙碌的，后 9ms 是空闲的。
    *   但我们在 `steadywin_motor.c` 里设置了 `MONITOR_INTERVAL_MS = 20`。
    *   这意味着：每发大概 **2~3 次** 速度指令后，`last_poll_time` 就会超时。
    *   此时 `sw_motor_process` 发现总线空闲，就会**见缝插针**地发一条 `0xB4 (Get Vbus)`。
    *   下一次控制周期，发一条 `0xB2 (Get Error)`。

### 总结
你不需要显式调用 `read_voltage()`。你只管发控制指令，或者什么都不做。只要总线有空，驱动就会自动把 `m1.cache.v_bus` 和 `m1.cache.fault_flags` 填好供你读取。

但是要注意的是，有FIFO深度限制，如果同一个函数里面疯狂push 而不给pop留时间的话，会导致队列满溢。（可以在debug界面关注tx tail和rx tail）看看数据处理是否跟的上数据接收。如果跟不上，就要考虑你的业务逻辑是否合理了。



### 待优化：

#### A.队列需要增加优先级机制

在电机控制中，**“设置参数”需要排队，但“速度/位置指令”需要插队或覆盖**。

- **问题**：如果CAN总线拥堵，你的 queue 满了，新的控制指令（例如最新的位置环计算结果）会被 _queue_push 返回 E_FULL 丢弃。但实际上，对于控制指令，旧的指令已经没意义了，我们只想要执行最新的。
- **建议**：修改 _queue_push，增加一种“覆盖模式”。

#### B.OS优化

为os增加一个适配层，当使用RTOS编译时，利用信号量或者互斥锁，把异步变成同步。

裸机部分代码不要修改，仅增加一个阻塞的api适配层。