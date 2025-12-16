# 黄油滤波器组件

本目录提供了一个黄油滤波器（Butterworth Filter）的实现，用于信号处理和数据平滑。

## 文件结构

```
butter/
├── butter.c       # 黄油滤波器实现
├── butter.h       # 黄油滤波器头文件
└── mybutter.m     # Matlab脚本，用于生成滤波器系数
```

## 核心功能

- 提供低通、高通、带通和带阻四种滤波类型
- 支持不同的滤波阶数
- 提供浮点和定点实现（如果支持）
- 支持单通道和多通道信号处理

## 核心组件说明

### 1. butter.h

**功能**：黄油滤波器的头文件，包含所有公共API声明

**主要内容**：
- 滤波器类型和阶数定义
- 滤波器结构体定义
- 滤波器初始化和使用函数声明

### 2. butter.c

**功能**：黄油滤波器的实现文件，包含所有算法实现

**主要内容**：
- 滤波器系数计算
- 滤波算法实现
- 初始化和重置函数

### 3. mybutter.m

**功能**：Matlab脚本，用于生成滤波器系数

**主要内容**：
- 使用Matlab的butter函数生成系数
- 系数格式转换
- 导出系数到C代码

## 基本使用方法

### 1. 初始化滤波器

```c
#include "butter.h"

// 定义滤波器参数
static butter_params_t params = {
    .type = BUTTER_FILTER_LOWPASS,  // 低通滤波器
    .order = 2,                      // 2阶滤波器
    .cutoff_freq = 100.0,            // 截止频率100Hz
    .sample_freq = 1000.0            // 采样频率1000Hz
};

// 初始化滤波器
butter_filter_t *filter = butter_filter_init(&params);
```

### 2. 使用滤波器

```c
// 滤波单个样本
float input_sample = 1.0;
float output_sample = butter_filter_process(filter, input_sample);

// 滤波多个样本
float input_samples[] = {1.0, 2.0, 3.0, 4.0, 5.0};
float output_samples[5];

for (int i = 0; i < 5; i++) {
    output_samples[i] = butter_filter_process(filter, input_samples[i]);
}
```

### 3. 重置滤波器

```c
// 重置滤波器状态
butter_filter_reset(filter);
```

### 4. 释放滤波器

```c
// 释放滤波器资源
butter_filter_deinit(filter);
```

## 滤波器类型说明

| 类型 | 描述 |
| ---- | ---- |
| BUTTER_FILTER_LOWPASS | 低通滤波器，允许低频信号通过，衰减高频信号 |
| BUTTER_FILTER_HIGHPASS | 高通滤波器，允许高频信号通过，衰减低频信号 |
| BUTTER_FILTER_BANDPASS | 带通滤波器，允许指定频率范围内的信号通过 |
| BUTTER_FILTER_BANDSTOP | 带阻滤波器，衰减指定频率范围内的信号 |

## 滤波阶数选择

- 阶数越高，滤波效果越好，但计算复杂度越高
- 常用阶数：1-4阶
- 阶数选择应根据实际需求和系统计算能力平衡

## Matlab系数生成

使用`mybutter.m`脚本可以生成自定义滤波器系数：

```matlab
% 设置滤波器参数
order = 2;
cutoff_freq = 100;
sample_freq = 1000;
filter_type = 'low';

% 生成系数
[b, a] = butter(order, cutoff_freq/(sample_freq/2), filter_type);

% 打印系数
fprintf('B coefficients: ');
for i = 1:length(b)
    fprintf('%f, ', b(i));
end
fprintf('\n');

fprintf('A coefficients: ');
for i = 1:length(a)
    fprintf('%f, ', a(i));
end
fprintf('\n');
```

## 性能优化建议

1. 对于固定参数的滤波，可以预计算系数以提高性能
2. 对于实时应用，考虑使用定点实现
3. 对于多通道信号，可以复用同一个滤波器结构（注意状态重置）
4. 对于高频采样信号，考虑降低采样率后再滤波

## 应用场景

- 传感器信号处理（如温度、湿度、加速度等）
- 音频信号处理
- 控制系统反馈信号平滑
- 数据采集和分析