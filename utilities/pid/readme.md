# PID控制器组件

本目录提供了一个PID控制器的实现，支持位置式和增量式PID算法，适用于各种控制系统。

## 文件结构

```
pid/
├── pid.c    # PID控制器实现
├── pid.h    # PID控制器头文件
└── readme.md # 本说明文件
```

## 核心功能

- 支持浮点数和定点数两种实现
- 支持位置式PID算法
- 支持自动和手动模式切换
- 支持正反作用切换
- 支持输出限幅
- 支持采样时间配置
- 支持动态调整PID参数

## 核心组件说明

### 1. pid.h

**功能**：PID控制器的头文件，包含所有公共API声明

**主要内容**：
- PID控制器结构体定义（浮点型和定点型）
- PID控制算法函数声明
- 参数配置函数声明

### 2. pid.c

**功能**：PID控制器的实现文件，包含所有算法实现

**主要内容**：
- PID初始化和销毁
- PID计算算法实现
- 参数调整和模式切换

## 基本使用方法

### 1. 初始化PID控制器（浮点型）

```c
#include "pid.h"

// 创建PID控制器实例
PID_Controller *pid = pid_new(
    2.0,   // Kp比例系数
    0.5,   // Ki积分系数
    0.1,   // Kd微分系数
    100,   // 采样时间（ms）
    -100.0, // 输出最小值
    100.0   // 输出最大值
);
```

### 2. 设置模式和目标值

```c
// 设置为自动模式
pid_set_mode(pid, AUTOMATIC);

// 设置控制方向（DIRECT/REVERSE）
// pid->dir = DIRECT;

// 设置目标值
pid->Setpoint = 50.0;
```

### 3. 执行PID计算

```c
// 在控制循环中调用
while (1) {
    // 获取当前输入值
    pid->Input = get_feedback_value();
    
    // 执行PID计算
    pid_compute(pid);
    
    // 使用输出值控制执行机构
    set_output_value(pid->Output);
    
    // 等待采样时间
    delay(pid->SampleTime);
}
```

### 4. 动态调整参数

```c
// 调整PID参数
pid_set_tunings(pid, 3.0, 0.3, 0.2);

// 调整采样时间
pid_set_sampletime(pid, 50); // 50ms

// 调整输出限幅
pid_set_output_limits(pid, -50.0, 50.0);
```

### 5. 销毁PID控制器

```c
// 销毁PID控制器实例
pid_delete(pid);
pid = NULL;
```

### 6. 定点数PID使用（Q格式）

```c
// 创建定点数PID控制器实例
QPID_Controller *qpid = qpid_new(
    1024,  // Kp比例系数（Q10格式）
    16384, // Ki积分系数（Q14格式）
    -8192,  // 输出最小值（Q14格式）
    8192    // 输出最大值（Q14格式）
);

// 设置定点数PID参数
qpid->Setpoint = 4096; // 目标值（Q14格式）

// 执行定点数PID计算
while (1) {
    qpid->Input = get_q14_feedback_value();
    qpid_compute_raw(qpid);
    set_q14_output_value(qpid->Output);
    delay(100);
}
```

## PID控制器结构体说明

### 浮点型PID结构体

```c
typedef struct {
    int32_t lastTime;     // 上次计算时间
    double Input;         // 当前输入值
    double Output;        // 当前输出值
    double Setpoint;      // 目标值
    double ITerm;         // 积分项
    double lastInput;     // 上次输入值
    double kp;            // 比例系数
    double ki;            // 积分系数
    double kd;            // 微分系数
    int SampleTime;       // 采样时间（ms）
    double outMin;        // 输出最小值
    double outMax;        // 输出最大值
    int dir;              // 控制方向（DIRECT/REVERSE）
    bool inAuto;          // 自动/手动模式
} PID_Controller;
```

### 定点数PID结构体

```c
typedef struct {
    int16_t Input;        // 当前输入值（Q14）
    int16_t Output;       // 当前输出值（Q14）
    int16_t Setpoint;     // 目标值（Q14）
    int32_t Error;        // 误差（Q14）
    int32_t PTerm;        // 比例项（Q14）
    int32_t ITerm;        // 积分项（Q28）
    int16_t lastInput;    // 上次输入值
    int16_t kp;           // 比例系数（Q10）
    int16_t ki;           // 积分系数（Q14）
    int16_t outMin;       // 输出最小值（Q14）
    int16_t outMax;       // 输出最大值（Q14）
    bool inAuto;          // 自动/手动模式
} QPID_Controller;
```

## 应用场景

- 温度控制系统
- 电机速度控制
- 位置控制系统
- 压力控制系统
- 流量控制系统

## PID参数调整建议

1. **比例系数（Kp）**：
   - 增大Kp可以加快响应速度，但可能导致超调
   - 减小Kp可以减小超调，但响应速度变慢

2. **积分系数（Ki）**：
   - 增大Ki可以减小稳态误差，但可能导致振荡
   - 减小Ki可以减少振荡，但稳态误差增大

3. **微分系数（Kd）**：
   - 增大Kd可以减小超调和振荡，但可能导致噪声敏感
   - 减小Kd可以增加系统稳定性，但超调可能增大

## 注意事项

1. 确保采样时间设置合理，一般为系统时间常数的1/10到1/20
2. 输出限幅应根据执行机构的实际能力设置
3. 正反作用选择应根据控制系统的实际特性确定
4. 定点数PID需要注意数据格式和溢出问题
5. 在资源受限的系统上，定点数PID比浮点型PID更高效