/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_types.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 16:12:38
 * @Brief        : FOC 运行时类型定义
 */

#ifndef __FOC_TYPES_H__
#define __FOC_TYPES_H__

/*---------- includes ----------*/
#include "foc_math_types.h"
#include <stdbool.h>
#include <stdint.h>

/*---------- type define ----------*/
typedef enum {
    FOC_MODE_STOP = 0,
    FOC_MODE_CURRENT,
    FOC_MODE_SPEED,
    FOC_MODE_FAULT,
} foc_mode_t;

typedef enum {
    FOC_FAULT_NONE = 0x00000000UL,
    FOC_FAULT_INVALID_PROFILE = 0x00000001UL,
    FOC_FAULT_OVERCURRENT = 0x00000002UL,
} foc_fault_mask_t;

typedef enum {
    FOC_ANGLE_STATUS_NONE = 0,  /* 尚未建立有效角度，常见于上电、对齐前或角度源未初始化完成 */
    FOC_ANGLE_STATUS_VALID,     /* 角度来自当前可直接使用的测量值，默认视为当前版本最标准的可消费状态 */
    FOC_ANGLE_STATUS_PREDICTED, /* 角度基于最近一次有效测量和速度外推或插值得到，常用于异步传感器对齐电流采样时刻 */
    FOC_ANGLE_STATUS_ESTIMATED, /* 角度由观测器、PLL 或其他估算器给出，可用于无传感器或融合场景 */
    FOC_ANGLE_STATUS_INVALID,   /* 当前角度不可消费，具体退化或故障策略后续由状态机和保护层定义 */
} foc_angle_status_t;

typedef struct {
    foc_scalar_t a_real;
    foc_scalar_t b_real;
    foc_scalar_t bus_voltage;
    uint32_t sample_tick_us;
} foc_current_loop_sample_t;

typedef struct {
    /* 角度提供器返回的电角度估计 */
    foc_angle_t electrical_angle;  /* 对齐到目标时刻后的电角度结果 */
    foc_scalar_t electrical_speed; /* 与该角度配套的电角速度估计，可用于下一步预测或调试 */
    uint32_t angle_tick_us;        /* 当前角度样本自身对应的时间戳，便于判断外推跨度或数据新鲜度 */
    foc_angle_status_t status;     /* 角度状态，描述该结果是实测、预测、估算还是不可消费 */
} foc_angle_sample_t;

typedef struct {
    /* 角度提供器返回的机械角估计 */
    foc_angle_t mechanical_angle;  /* 对齐到目标时刻后的机械角度结果 */
    foc_scalar_t mechanical_speed; /* 与该角度配套的机械角速度估计 */
    uint32_t angle_tick_us;        /* 当前角度样本自身对应的时间戳 */
    foc_angle_status_t status;     /* 当前机械角是否可消费 */
} foc_mechanical_angle_sample_t;

typedef struct {
    /* 运行模式和故障状态 */
    foc_mode_t mode;
    uint32_t fault_mask;

    /* 关键观测量与给定量 */
    foc_angle_t electrical_angle;    /* 当前电流环实际使用的电角度 */
    foc_scalar_t electrical_speed;   /* 当前电流环实际使用的电角速度 */
    foc_angle_status_t angle_status; /* 当前电流环使用的角度状态 */
    foc_angle_t electrical_zero_offset; /* 运行时电角度零位偏移 */
    bool electrical_zero_valid;         /* 运行时电角度零位是否已经标定 */
    foc_scalar_t bus_voltage_pu;     /* 当前电流环使用的母线电压标幺值 */
    uint32_t current_sample_tick_us; /* 电流与母线电压样本的同步时间戳 */
    uint32_t angle_sample_tick_us;   /* 角度样本自身的时间戳，可能早于或等于 current_sample_tick_us */
    foc_scalar_t speed_ref;          /* 速度参考标幺值 */
    foc_scalar_t speed_feedback;     /* 速度反馈标幺值 */

    /* 电流环与调制链路中间量 */
    foc_dq_t current_ref_dq;  /* d-q 电流参考标幺值 */
    foc_dq_t current_meas_dq; /* d-q 电流反馈标幺值 */
    foc_dq_t voltage_cmd_dq;  /* d-q 电压指令标幺值 */
    foc_ab_t voltage_cmd_ab;  /* alpha-beta 电压指令标幺值 */
    foc_pwm_duty_t pwm_duty;
} foc_runtime_t;

#endif
