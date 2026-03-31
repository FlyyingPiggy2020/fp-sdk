/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : pmsm_foc.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 11:14:25
 * @Brief        : PMSM FOC 控制器骨架
 */

#ifndef __PMSM_FOC_H__
#define __PMSM_FOC_H__

/*---------- includes ----------*/
#ifdef FOC_INCLDUE_BOARD_OPTIONS_FILE
#include "foc_options.h"
#endif
#include "foc_cfg.h"
#include "foc_log.h"

#include "foc_pi.h"
#include "foc_hal.h"
#include "foc_profile.h"
#include <stdbool.h>
/*---------- macro ----------*/

/*---------- type define ----------*/
typedef struct {
    /* 配置对象由业务层或板级层持有，控制器只引用 */
    const motor_profile_t *motor_profile;
    const power_stage_profile_t *power_stage_profile;
    const sensor_profile_t *sensor_profile;
    const foc_ctrl_cfg_t *ctrl_cfg;

    /* 板级适配接口 */
    foc_hal_ops_t hal_ops;
    void *hal_user_data;

    /* 各环控制器 */
    foc_pi_t id_pi;
    foc_pi_t iq_pi;
    foc_pi_t speed_pi;

    /* 实时运行状态 */
    foc_runtime_t runtime;
} pmsm_foc_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化 PMSM FOC 控制器
 * @param  foc: 控制器对象
 * @param  motor_profile: 电机参数
 * @param  power_stage_profile: 功率板参数
 * @param  sensor_profile: 传感器参数
 * @param  ctrl_cfg: 控制器配置
 * @param  hal_ops: 板级硬件操作接口
 * @param  hal_user_data: 板级私有上下文
 * @return true=成功, false=失败
 */
bool pmsm_foc_init(pmsm_foc_t *foc,
                   const motor_profile_t *motor_profile,
                   const power_stage_profile_t *power_stage_profile,
                   const sensor_profile_t *sensor_profile,
                   const foc_ctrl_cfg_t *ctrl_cfg,
                   const foc_hal_ops_t *hal_ops,
                   void *hal_user_data);

/**
 * @brief  启动 FOC 控制器
 * @param  foc: 控制器对象
 * @param  mode: 启动模式
 * @return 无
 */
void pmsm_foc_start(pmsm_foc_t *foc, foc_mode_t mode);

/**
 * @brief  停止 FOC 控制器
 * @param  foc: 控制器对象
 * @return 无
 */
void pmsm_foc_stop(pmsm_foc_t *foc);

/**
 * @brief  设置 d-q 电流给定
 * @param  foc: 控制器对象
 * @param  id_ref: d 轴电流给定
 * @param  iq_ref: q 轴电流给定
 * @return 无
 */
void pmsm_foc_set_current_ref(pmsm_foc_t *foc, foc_scalar_t id_ref, foc_scalar_t iq_ref);

/**
 * @brief  设置速度给定
 * @param  foc: 控制器对象
 * @param  speed_ref: 速度参考值
 * @return 无
 */
void pmsm_foc_set_speed_ref(pmsm_foc_t *foc, foc_scalar_t speed_ref);

/**
 * @brief  设置运行时电角度零位偏移
 * @param  foc: 控制器对象
 * @param  electrical_zero_offset: 电角度零位偏移
 * @return 无
 */
void pmsm_foc_set_electrical_zero_offset(pmsm_foc_t *foc, foc_angle_t electrical_zero_offset);

/**
 * @brief  清除运行时电角度零位偏移有效标志
 * @param  foc: 控制器对象
 * @return 无
 */
void pmsm_foc_clear_electrical_zero_offset(pmsm_foc_t *foc);

/**
 * @brief  直接输出固定电压矢量
 * @param  foc: 控制器对象
 * @param  voltage_cmd_dq: d-q 电压指令标幺值
 * @param  electrical_angle: 输出使用的电角度
 * @param  enable_output: true=输出前使能功率级, false=保持当前使能状态
 * @return true=执行成功, false=执行失败
 */
bool pmsm_foc_apply_voltage_vector(pmsm_foc_t *foc,
                                   const foc_dq_t *voltage_cmd_dq,
                                   foc_angle_t electrical_angle,
                                   bool enable_output);

/**
 * @brief  执行一次电流环控制
 * @param  foc: 控制器对象
 * @return true=执行成功, false=执行失败
 */
bool pmsm_foc_current_loop(pmsm_foc_t *foc);

/**
 * @brief  执行一次速度环控制
 * @param  foc: 控制器对象
 * @return 无
 */
void pmsm_foc_speed_loop(pmsm_foc_t *foc);
/*---------- end of file ----------*/
#endif
