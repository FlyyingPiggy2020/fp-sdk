/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_profile.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 16:15:39
 * @Brief        : 电机与 FOC 参数配置定义
 */

#ifndef __FOC_PROFILE_H__
#define __FOC_PROFILE_H__

/*---------- includes ----------*/
#include "../foc_types.h"
#include "../math/foc_math_types.h"
#include <stdbool.h>
#include <stdint.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct {
    /* 电机本体参数 */
    uint8_t pole_pairs;            /* 极对数，用于机械角速度与电角速度换算 */
    int8_t electrical_direction;   /* 电角度方向，通常取 1 或 -1，用于统一旋转方向定义 */
    foc_angle_t electrical_offset; /* 电角度零点偏移，用于将传感器原始角度对齐到控制坐标系 */
    foc_scalar_t inv_i_base;       /* 电流基准值的倒数，单位 A，用于标幺化电流和参数 */
    foc_scalar_t inv_v_base;       /* 电压基准值的倒数，单位 V，用于标幺化电压和参数 */
} motor_profile_t;

typedef struct {
    /* 功率板与采样链参数 */
    uint32_t pwm_frequency_hz;       /* PWM 载波频率，单位 Hz，决定电流环基频和调制周期 */
    uint32_t deadtime_ns;            /* 上下桥臂死区时间，单位 ns，用于驱动时序补偿或参数记录 */
    foc_scalar_t bus_voltage_ratio;  /* 母线分压比例或还原系数，用于采样值恢复母线实际电压 */
    foc_scalar_t over_current_limit; /* 硬件或软件过流阈值，用于保护判断和输出关断 */
} power_stage_profile_t;

typedef struct {
    /* 传感器与采样通道参数 */
    uint16_t encoder_cpr;          /* 编码器每机械转一圈的计数数，含线数换算后的最终分辨率 */
    int8_t encoder_direction;      /* 编码器计数方向，通常取 1 或 -1，用于统一角度正方向 */
    foc_angle_t electrical_offset; /* 传感器对应的电角度零点偏移，可与电机对齐结果绑定 */
    foc_scalar_t current_a_offset; /* A 相电流 ADC 零偏码值，用于去除运放与 ADC 的直流偏置 */
    foc_scalar_t current_b_offset; /* B 相电流 ADC 零偏码值，用于去除运放与 ADC 的直流偏置 */
    foc_scalar_t current_c_offset; /* C 相电流 ADC 零偏码值，三相采样场景下用于去偏 */
} sensor_profile_t;

typedef struct {
    /* 控制器整定与环路频率参数 */
    foc_scalar_t id_kp_pu;         /* d 轴电流环比例系数，决定 d 轴误差的快速响应强度 */
    foc_scalar_t id_ki_pu;         /* d 轴电流环积分系数，用于消除 d 轴稳态误差 */
    foc_scalar_t iq_kp_pu;         /* q 轴电流环比例系数，决定转矩通道的快速响应强度 */
    foc_scalar_t iq_ki_pu;         /* q 轴电流环积分系数，用于消除 q 轴稳态误差 */
    foc_scalar_t speed_kp_pu;      /* 速度环比例系数，用于速度误差到转矩给定的快速调节 */
    foc_scalar_t speed_ki_pu;      /* 速度环积分系数，用于消除速度稳态误差 */
    foc_scalar_t voltage_limit_pu; /* 电压矢量输出限幅，通常与母线利用率和调制范围对应 */
    uint32_t current_loop_hz;      /* 电流环执行频率，通常对应 PWM 中断频率 */
    uint32_t speed_loop_hz;        /* 速度环执行频率，通常对应低频调度周期 */
} foc_ctrl_cfg_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化电机本体参数为默认值
 * @param  profile: 电机参数对象
 * @return 无
 */
void motor_profile_init(motor_profile_t *profile);

/**
 * @brief  初始化功率板参数为默认值
 * @param  profile: 功率板参数对象
 * @return 无
 */
void power_stage_profile_init(power_stage_profile_t *profile);

/**
 * @brief  初始化传感器参数为默认值
 * @param  profile: 传感器参数对象
 * @return 无
 */
void sensor_profile_init(sensor_profile_t *profile);

/**
 * @brief  初始化控制器参数为默认值
 * @param  cfg: 控制器配置对象
 * @return 无
 */
void foc_ctrl_cfg_init(foc_ctrl_cfg_t *cfg);

/**
 * @brief  校验电机本体参数是否合法
 * @param  profile: 电机参数对象
 * @return true=合法, false=非法
 */
bool motor_profile_is_valid(const motor_profile_t *profile);

/**
 * @brief  校验功率板参数是否合法
 * @param  profile: 功率板参数对象
 * @return true=合法, false=非法
 */
bool power_stage_profile_is_valid(const power_stage_profile_t *profile);

/**
 * @brief  校验传感器参数是否合法
 * @param  profile: 传感器参数对象
 * @return true=合法, false=非法
 */
bool sensor_profile_is_valid(const sensor_profile_t *profile);

/**
 * @brief  校验控制器参数是否合法
 * @param  cfg: 控制器配置对象
 * @return true=合法, false=非法
 */
bool foc_ctrl_cfg_is_valid(const foc_ctrl_cfg_t *cfg);
/*---------- end of file ----------*/
#endif
