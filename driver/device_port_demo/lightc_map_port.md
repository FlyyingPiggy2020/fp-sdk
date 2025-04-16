# 介绍

`lightc_map`是一个根据亮度-占空比表格和亮度-频率表格去查找，并且线性插值。从而根据不同的亮度输出对应的占空比和频率的驱动程序。

所有的参数都在`lightc_map_describe_t`结构体的内部。其中`priv`开头的为私有变量。`param`为可以调整的参数。

支持调整渐亮时间，渐灭时间。支持循环调光等API。

> [!NOTE]
>
> 如果要调光柔和的话，最好MCU主频要高，保证ARR值尽可能的大，这样子才能对占空比有更高的细分。
>
> 但是ARR越大，在CLOCK不变的情况下，频率会变小。频率越小波动深度会越大。
>
> 所以波动深度和柔和效果是两个矛盾的参数，一个增加另一个会减小。如何平衡他们两者。最好的办法就是换一个主频高的MCU。
>
> 如果MCU主频不够高的话，需要驱动同时改变调整占空比和频率。



# 移植说明

1. 实现一个时间片

```
static device_t *dev_light = NULL;
void task_light_init(void)
{
	dev_light = device_open("light");
	device_ioctl(dev_light, IOCTL_LIGHTC_START, NULL);
}
```

2. 在轮询中调用*device_irq_process*，下面这个例子就是在1ms的时间片中调用它

```
void task_light_poll(void)
{
    static uint32_t last_time = 0;
    if (bsp_CheckRunTime(last_time) >= 1) {
        last_time = bsp_GetRunTime();
        device_irq_process(dev_light, NULL, NULL, 0);
    }
}
```

3. 实现port接口


```c
/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : bsp_light_map.c
 * @Author       : lxf
 * @Date         : 2024-12-06 16:08:25
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2025-04-15 14:44:42
 * @Brief        : 
 * 实现lightc_map_describe_t ops 这个接口，使用DEVICE_DEFINED这个宏定义，将ops注册到设备管理器中。
 * 下面给出了一份示例，依赖于pwmc这个pwm输出组件(也可以替换成你自己的pwm输出方式)
 */

/*---------- includes ----------*/
#include "stdbool.h"
#include "pwmc.h"
#include "lightc_map.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool _init(void);
static void _deinit(void);
static int32_t _update_brightness(uint32_t frequence, float duty);
/*---------- variable ----------*/

/**
 * @brief 亮度map(matlab生成)
 * @param {double} bmap_node.x 亮度值 [0,100]
 * @param {double} bmap_node.y 亮度对应的PWM占空比 [0,1]
 * @return {*}
 */
static _bmap_node_t bmap_node[] = {
    { 0.000000, 0.000000 },
    { 1.000000, 0.000090 },
    { 2.000000, 0.000196 },
    { 3.000000, 0.000318 },
    { 4.000000, 0.000461 },
    { 5.000000, 0.000626 },
    { 6.000000, 0.000819 },
    { 7.000000, 0.001043 },
    { 8.000000, 0.001304 },
    { 9.000000, 0.001607 },
    { 10.000000, 0.001960 },
    { 11.000000, 0.002370 },
    { 12.000000, 0.002846 },
    { 13.000000, 0.003400 },
    { 14.000000, 0.004044 },
    { 15.000000, 0.004792 },
    { 16.000000, 0.005661 },
    { 17.000000, 0.006670 },
    { 18.000000, 0.007842 },
    { 19.000000, 0.009202 },
    { 20.000000, 0.010780 },
    { 21.000000, 0.012610 },
    { 22.000000, 0.014731 },
    { 23.000000, 0.017187 },
    { 24.000000, 0.020029 },
    { 25.000000, 0.023316 },
    { 26.000000, 0.027113 },
    { 27.000000, 0.031495 },
    { 28.000000, 0.036543 },
    { 29.000000, 0.042352 },
    { 30.000000, 0.049024 },
    { 31.000000, 0.056671 },
    { 32.000000, 0.065415 },
    { 33.000000, 0.075389 },
    { 34.000000, 0.086729 },
    { 35.000000, 0.099580 },
    { 36.000000, 0.114086 },
    { 37.000000, 0.130387 },
    { 38.000000, 0.148617 },
    { 39.000000, 0.168890 },
    { 40.000000, 0.191298 },
    { 41.000000, 0.215898 },
    { 42.000000, 0.242704 },
    { 43.000000, 0.271678 },
    { 44.000000, 0.302723 },
    { 45.000000, 0.335674 },
    { 46.000000, 0.370303 },
    { 47.000000, 0.406316 },
    { 48.000000, 0.443363 },
    { 49.000000, 0.481049 },
    { 51.000000, 0.518951 },
    { 52.000000, 0.556637 },
    { 53.000000, 0.593684 },
    { 54.000000, 0.629697 },
    { 55.000000, 0.664326 },
    { 56.000000, 0.697277 },
    { 57.000000, 0.728322 },
    { 58.000000, 0.757296 },
    { 59.000000, 0.784102 },
    { 60.000000, 0.808702 },
    { 61.000000, 0.831110 },
    { 62.000000, 0.851383 },
    { 63.000000, 0.869613 },
    { 64.000000, 0.885914 },
    { 65.000000, 0.900420 },
    { 66.000000, 0.913271 },
    { 67.000000, 0.924611 },
    { 68.000000, 0.934585 },
    { 69.000000, 0.943329 },
    { 70.000000, 0.950976 },
    { 71.000000, 0.957648 },
    { 72.000000, 0.963457 },
    { 73.000000, 0.968505 },
    { 74.000000, 0.972887 },
    { 75.000000, 0.976684 },
    { 76.000000, 0.979971 },
    { 77.000000, 0.982813 },
    { 78.000000, 0.985269 },
    { 79.000000, 0.987390 },
    { 80.000000, 0.989220 },
    { 81.000000, 0.990798 },
    { 82.000000, 0.992158 },
    { 83.000000, 0.993330 },
    { 84.000000, 0.994339 },
    { 85.000000, 0.995208 },
    { 86.000000, 0.995956 },
    { 87.000000, 0.996600 },
    { 88.000000, 0.997154 },
    { 89.000000, 0.997630 },
    { 90.000000, 0.998040 },
    { 91.000000, 0.998393 },
    { 92.000000, 0.998696 },
    { 93.000000, 0.998957 },
    { 94.000000, 0.999181 },
    { 95.000000, 0.999374 },
    { 96.000000, 0.999539 },
    { 97.000000, 0.999682 },
    { 98.000000, 0.999804 },
    { 99.000000, 0.999910 },
    { 100.000000, 1.000000 },
};

static _fmap_node_t fmap_node[] = {
    { 1200, 0},
    { 1200, 33},
    { 3125, 100},
};

static brightness_map_t bmap = {
    .node = bmap_node,
    .node_size = sizeof(bmap_node) / sizeof(bmap_node[0]),
};

static frequenct_map_t fmap = {
    .node = fmap_node,
    .node_size = sizeof(fmap_node) / sizeof(fmap_node[0]),
};


lightc_map_describe_t ops = {
    .bmap = &bmap, // 亮度map
    .fmap = &fmap, // 频率map 
    .time_slice_frequence = 1000, //执行loop时间片的频率;单位hz
    .ops = {
        .init = _init,//初始化
        .deinit = _deinit,//反初始化
        .update_brightness = _update_brightness,//同时更新亮度和频率
    },
    .priv = {
        .brightness_position = 0, //目标亮度位置
    },
};

device_t *brightness = NULL;
device_t *current = NULL;

/*---------- function ----------*/
DEVICE_DEFINED(light, lightc_map, &ops);

static bool _init(void)
{
    brightness = device_open("brightness");//这里的brightness是我的pwmc设备的名称。
    device_ioctl(brightness, IOCTL_PWMC_ENABLE, NULL);
    return true;
}
static void _deinit(void)
{
    device_close(brightness);
}

/**
 * @brief 同时更新亮度和频率
 * @param {uint32_t} frequence 频率 
 * @param {float} duty 占空比[0,1]
 * @return {*}
 */
static int32_t _update_brightness(uint32_t frequence, float duty)
{
    struct pwmc_ioctl_param param = {
        .freq = frequence,
        .duty = duty,
    };
    return device_ioctl(brightness, IOCTL_PWMC_SET_FREQ_DUTY, &param);
}
/*---------- end of file ----------*/

```

4. matlab生成对应数组的程序(我用的版本是MATLAB R2022B，其他版本应该是通用的)

```matlab
% 伽马调光曲线生成器（亮度从0.2开始）
clear; close all; clc;

% 参数设置
gamma = 15;           % 伽马值
resolution = 100;       % 输入分辨率
start_brightness = 0.0066; % 起始亮度
k = 15; % Sigmoid函数的斜率

% 生成伽马曲线
% 计算伽马曲线的起始点和线性曲线的起始点
a = start_brightness^(1/gamma);
x = linspace(0, 1, resolution);
x_remapped = a + x*(1 - a);      % 输入信号偏移
y_gamma = x_remapped.^gamma;

% 生成S形曲线
y_sigmoid = 1 ./ (1 + exp((-k)*(x - 0.5)));
y0 = 1 / (1 + exp(-k*(0 - 0.5))); % x = 0
y1 = 1 / (1 + exp(-k*(1 - 0.5))); % x = 1
y_sigmoid_adjusted = (y_sigmoid - y0) / (y1 - y0);
% 生成S形曲线(前半截)
y_sigmoid_half_raw = 1 ./ (1 + exp(-k*(x/2 - 0.5)));
y_min_half = 1 / (1 + exp(-k*(0 - 0.5)));    % 当x=0，此处值较小
y_max_half = 1 / (1 + exp(-k*(0.5 - 0.5)));    % 当x=0.5，此值为0.5
y_sigmoid_half = (y_sigmoid_half_raw - y_min_half) / (y_max_half - y_min_half);


% 可视化
figure('Name', '调光曲线');
plot(x*100, y_gamma, 'LineWidth', 2);  % X轴显示为百分比
hold on;
plot(x*100, y_sigmoid_adjusted, '--', 'LineWidth', 2); % 添加S形曲线
plot(x*100, y_sigmoid_half, '-.', 'LineWidth', 2);
title(['偏移伽马曲线与S形曲线 (\gamma = ', num2str(gamma), ')']);
xlabel('输入信号 (%)');
ylabel('输出物理亮度');
legend('伽马曲线', 'S形曲线', 'S形曲线上半截', 'Location', 'northwest');
grid on;
axis([0 100 0 1.1]);

% % 生成占空比-亮度查找表
% for i = 1:length(x)
%     fprintf('{%f, %f},\n', round(x(i)*100), y_gamma(i));
% end
% 
disp('S形曲线占空比-亮度查找表:');
for i = 1:length(x)
    fprintf('{%f, %f},\n', round(x(i)*100), y_sigmoid_adjusted(i));
end
% % 输出S形半截的占空比-亮度查找表
% disp('S形曲线加速递增半截占空比-亮度查找表:');
% for i = 1:length(x)
%     fprintf('{%f, %f},\n', round(x(i)*100), y_sigmoid_half(i));
% end
```

# API说明

## IOCTL_LIGHTC_SET_CURRENT_MODE
```c
device_ioctl(dev_light, IOCTL_LIGHTC_SET_CURRENT_MODE, NULL);
```
暂时未使用，原计划调整恒流芯片限流，后来没有用上。
## IOCTL_LIGHTC_CMD_OFF
```c
device_ioctl(dev_light, IOCTL_LIGHTC_CMD_OFF, NULL);
```
灯光关闭(亮度到0%)
## IOCTL_LIGHTC_CMD_OFF
```c
device_ioctl(dev_light, IOCTL_LIGHTC_CMD_OFF, NULL);
```
灯光打开(亮度到上一次亮度)。

"上一次的亮度"为结构体`lightc_map_describe_t`中的`priv`的`remeber_brightness`成员。

它的保存逻辑为：每次调光完成后，如果亮度不为0，则会记住当前的亮度。下次操作IOCTL_LIGHTC_CMD_ON命令的时候，会将亮度调到`remeber_brightness`。

## IOCTL_LIGHTC_SET_BRIGHTNESS
```c
// 将亮度调到50%
struct lightc_map_param param;
param.brightness = 50;
device_ioctl(dev_light, IOCTL_LIGHTC_SET_BRIGHTNESS, &param);
```
将亮度调到50%（执行时间为默认时间）
## IOCTL_LIGHTC_STEP_BRIGHTNESS_INC
```c
device_ioctl(dev_light, IOCTL_LIGHTC_STEP_BRIGHTNESS_INC, NULL);
```
亮度单步递增(亮度在原来基础上增加5%)

## IOCTL_LIGHTC_SETP_BRIGHTNESS_DEC
```c
device_ioctl(dev_light, IOCTL_LIGHTC_SETP_BRIGHTNESS_DEC, NULL);
```
亮度单步递减(亮度在原来基础上减少5%)
## IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_INC
```c
device_ioctl(dev_light, IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_INC, NULL);
```
亮度连续递增(亮度一直增加到100%)

## IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_DEC
```c
device_ioctl(dev_light, IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_DEC, NULL);
```
亮度连续递减(亮度一直减少到0%)
## IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH
```c
device_ioctl(dev_light, IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH, NULL);
```
亮度调整结束

## IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START
```c
device_ioctl(dev_light, IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START, NULL);
```
循环调光开始(亮度从0%到100%，从100到0%，不断的循环执行)

## IOCTL_LIGHTC_LOOP_LIGHT_ADJ_STOP
```c
device_ioctl(dev_light, IOCTL_LIGHTC_LOOP_LIGHT_ADJ_STOP, NULL);
```
循环调光结束(和IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH这个API实际上效果一样)
## IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME
```c
// 将亮度调到50%，执行时间1秒
struct lightc_map_param param;
param.brightness = 50;
param.move_time = 1;//调到该亮度所需要的时间，单位秒
device_ioctl(dev_light, IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME, &param);
```
TASK_BELOW_LIGHT_SET_BRIGHTNESS的扩展版，支持修改执行时间。

## IOCTL_LIGHTC_START
```c
device_ioctl(dev_light, IOCTL_LIGHTC_START, NULL);
```
当修改了参数后需要调用这个API，去重新计算一些内部的私有参数
## IOCTL_LIGHTC_PARAM_READ
```c
device_ioctl(dev_light, IOCTL_LIGHTC_PARAM_READ, NULL);
```
参数读取，暂未实现，没想到好的API接口方式。我目前直接读的全局变量。

## IOCTL_LIGHTC_PARAM_WRITE
```c
void task_update_lightc(uint8_t start, uint8_t size, uint8_t *buf)
{
    struct lightc_map_param param = { 0 };
    param.param.start = start;
    param.param.size = size;
    memcpy(&(((uint8_t *)(&param.param.param.light_type))[start]), buf, size);
    device_ioctl(dev_light, IOCTL_LIGHTC_PARAM_WRITE, &param);
    task_storage_save(DATA_LIGHT_INDEX, 1);
}
```
参数写入。支持多个参数一起写入。写入方式是这样的。感觉不是很方便，可能后期会修改。
start，参数的起始位置。
size，需要写入的参数的个数。

## IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START_BY_TIME
```c
param.move_time = 1;//单位为1
device_ioctl(dev_light, IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START_BY_TIME, NULL);
```
循环调光开始(亮度从0%到100%，从100到0%，不断的循环执行，执行时间为move_time)



# 参数说明

```c
typedef struct {
    brightness_map_t *bmap;
    frequenct_map_t *fmap;
    double brightness;             //[0,100] for example 50 means 50%
    uint16_t time_slice_frequence; // default 100; unit:hz;

    struct {
        uint8_t light_type;          // default:0xff; reserve
        uint8_t dimming_start_point; // default:0;    unit:%
        uint8_t dimming_end_point;   // default:100;  unit:%
        uint8_t cut_start_point;     // default:0;    unit:%
        uint8_t cut_end_pint;        // default:100;  unit:%
        uint8_t start_delay;         // default:8;    unit:100ms (the time it taskes for brightness to go form 0% to 1%)
        uint8_t stop_delay;          // default:8;    unit:100ms (the time it taskes for brightness to go form 1% to 0%)
        uint8_t charge_duty;         // default:20;   unit:% (charge crr is [1% brightness crr value * charge duty])
        uint8_t fade_in_time;        // default:8;    unit:second (the time it taskes for brightness to go form 1% to 100%)
        uint8_t fade_out_time;       // default:8;    unit:second (the time it taskes for brightness to go form 100% to 1%)
        uint8_t start_state;         // default:0;    unit:0->off 1->on
    } param;

    struct {
        lightc_status_e status;
        lightc_status_e last_status;
        lightc_mode_e mode;
        uint8_t remeber_brightness; // default:100  device will remeber last brightness.the cmd "light on" means change the brightness to "remeber brightness".
        double brightness_position;
        uint32_t frequence;
        float duty;
        float brightness_actual;             //[0, 100]total brightness means the brightness without limit by "dimming_start_point" and "dimming_end_point"
        float brightness_step_1_percent_inc; // if brightness below to 1%,brightness change per 10ms.
        float brightness_step_1_percent_dec;
        float brightness_step_1_to_100_inc;
        float brightness_step_1_to_100_dec;
        float step_temp; // for IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME API
    } priv;

    struct {
        bool is_off;
    } status;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        int32_t (*update_brightness)(uint32_t frequence, float duty);
    } ops;
    struct {
        void (*lightc_stop_callback)(void); // when dimming stops,"lightc_stop_callback" will be called.
    } cb;
} lightc_map_describe_t;
```

目前所有的参数都在结构体`lightc_map_describe_t`里面。

`bmap`和`fmap`是亮度表格和频率表格，供查找使用。

brightness是显示亮度。

`time_slice_frequence`是执行`device_irq_process(dev_light, NULL, NULL, 0)`的频率。

## param

其中成员结构体`param`是参数。目前light_type，cut_start_point，cut_end_pint为预留参数，暂时不起到作用。

dimming_start_point是起始亮度(显示亮度为1%时的真实亮度)

dimming_end_point是结束亮度(显示亮度为100%时的真实亮度)

start_delay是启动延时（亮度从0%到1%的时间，单位为毫秒）

stop_delay是停止延时（亮度从1%到0%的时间，单位为毫秒）

charge_duty是充电占空比（）

fade_in_time是渐亮时间（亮度从1%到100%的时间，单位为秒）

fade_out_time是渐灭时间（亮度从100%到1%的时间，单位为秒）

start_state是上电后灯的状态

## status

这里是所有的状态

is_off 表示灯是打开的还是关闭的。

## ops

需要外部提供的操作接口

`update_brightness`更新PWM频率和占空比的接口，占空比为[0,1]闭区间。

## cb

提供给外部的回调函数

`lightc_stop_callback` 当真实的亮度和设置的目标相等的时候，会调用1次这个回调函数。（可以告诉别人我调光结束了）
