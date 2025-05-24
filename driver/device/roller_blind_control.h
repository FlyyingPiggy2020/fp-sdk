/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : roller_blind_control.h
 * @Author       : lxf
 * @Date         : 2025-05-14 15:36:45
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-20 08:32:31
 * @Brief        : 卷帘，百叶帘的驱动
 */

#ifndef __ROLLER_BLIND_CONTROL_H__
#define __ROLLER_BLIND_CONTROL_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/
#define IOCTL_MOTORC_OPEN           (IOCTL_USER_START + 0x00) // 开(点动，连续)
#define IOCTL_MOTORC_CLOSE          (IOCTL_USER_START + 0x01) // 关(点动，连续)
#define IOCTL_MOTORC_STOP           (IOCTL_USER_START + 0x02) // 停
#define IOCTL_MOTORC_GOTO_ROUTE     (IOCTL_USER_START + 0x03) // 百分比
#define IOCTL_MOTORC_SET_UP_ROUTE   (IOCTL_USER_START + 0x04) // 设置上行程
#define IOCTL_MOTORC_SET_DOWN_ROUTE (IOCTL_USER_START + 0x05) // 设置下行程
#define IOCTL_MOTORC_CLEAR_ROUTE    (IOCTL_USER_START + 0x06) // 清除行程
#define IOCTL_MOTORC_TURN           (IOCTL_USER_START + 0x07) // 开停关停
#define IOCTL_MOTORC_GET_STATUS     (IOCTL_USER_START + 0x08) // 获取电机状态
#define IOCTL_MOTORC_ROUTE_IS_FREE  (IOCTL_USER_START + 0x09) // 电机是否存在行程
#define IOCTL_MOTORC_GET_ROUTE      (IOCTL_USER_START + 0x0A) // 获取电机行程

#define MOTOR_ROUTE_FREE            0x7fffffff
#define MOTOR_ROUTE_INC_MAX         (0 + (MOTOR_ROUTE_FREE - 1))
#define MOTOR_ROUTE_DEC_MAX         (0 - (MOTOR_ROUTE_FREE - 1))

#define MFLAG_RESISTANCE_INC        (1 << 1) // inc过程中遇阻
#define MFLAG_RESISTANCE_DEC        (1 << 2) // dec过程中遇阻
#define MFLAG_RESISTANCE_SLOW       (1 << 3) // 减速遇阻
#define MFLAG_RESISTANCE_STOP       (1 << 4) // 堵转遇阻

#define MTURN_NORMAL                0x00
#define MTURN_OPEN_STOP             0x01
#define MTURN_CLOSE_STOP            0x02
#define MTURN_OPEN_CLOSE            0x03

#define MSTATE_STOP                 0 // 停止
#define MSTATE_RUN_INC              1 // 正转
#define MSTATE_RUN_DEC              2 // 反转

#define MMODE_NORMAL                (1 << 0) // 正常
#define MMODE_SSTEP                 (1 << 2) // 点动
#define MMODE_STOP_IFRUN            (1 << 3) // 如果电机正在运行，则停止，否则执行对应的动作
#define MMODE_OVERROUTE             (1 << 4) // 允许超出行程

// default
#define MOTOR_SPACE_MIN             (10)  // 默认允许运行的最小行程
#define MOTOR_SWITCH_TIME           (500) // 正反切换延时。单位毫秒
// // #define MOTOR_PW_MAX          (300 ms) // 允许的最大脉冲宽度（超过此宽度认为遇阻）
// #define MOTOR_PW_DMAX         0x40     // 允许的脉宽最大的变化系数（超过此值认为遇阻)

// #define preMax                100

// #define JOG_HALL              20  // 点动霍尔数
// #define MOTOR_ROUTE_MIN       300 // 行程最小值

// #define MIN_SPEED             20.0f // 最小速度 单位hz
// #define MAX_SPEED             130.0f // 最大速度 单位hz
// #define SLOW_SPEED_ROUTE      150    // 慢启慢停所需要的行程 单位：霍尔数
// #define SLOW_SPEED_D          5     // 变化的速度 单位：hz
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
#pragma pack(1)
typedef struct {
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*save_config)(void); // 保存电机config内的数据到非易失性存储器
        bool (*load_config)(void); // 从非易失性数据存储器内读取电机的config数据
        void (*motor_stop)(void);  // 电机停止
        void (*motor_inc)(void);   // 电机正转
        void (*motor_dec)(void);   // 电机反转
        uint16_t (*get_motor_current_with_offset)(void); //获取电机电流的adc值
    } ops;

    struct {
        int32_t route_curr; // 当前行程
        int32_t route_up;   // 上行程
        int32_t route_down; // 下行程
    } config;

    struct {
        struct {
            uint8_t state_curr;         // 当前状态（逻辑）
            uint8_t is_running;         // 当前状态（电机）
            uint8_t last_event;         // 上次状态，用于开停关停
            uint16_t disable_excloop;   // 开停关停循环之间的死区时间
            uint16_t start_update_time; // 5秒间隔上报计时
        } state;

        struct {
            int32_t target_route;     // 目标行程
            int32_t route_start;      // 停止时记录行程
            uint16_t time_stop_delay; // 当正反向切换时，继电器需要的延时时间。单位ms
            uint8_t is_jog;           // 点动模式
        } control;

        struct {
            uint8_t state;      // 遇阻状态(哪个方向遇阻)
            uint32_t run_time;  // 运行时间(电机运行5分钟需要保护)
            bool is_resistance; // 已遇阻 (过流或刹车或一切需要电机立即停止的标志位)
        } flag;
    } priv;

    struct {
        uint32_t space_min;
        uint16_t time_stop_delay;
    } param;
} roller_blind_control_describe_t;

union roller_blind_control_param {
    struct {
        uint8_t status;
        uint8_t route;
        bool is_route_free;
    }get;
};
#pragma pack()
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__ROLLER_BLIND_CONTROL_H__
