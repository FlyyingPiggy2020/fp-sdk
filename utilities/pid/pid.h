/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pid.h
 * @Author       : lxf
 * @Date         : 2024-12-02 08:21:57
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-26 14:13:53
 * @Brief        :
 */
#ifndef __PID_H__
#define __PID_H__
/*---------- includes ----------*/
#include <stdbool.h>
#include <stdint.h>
/*---------- macro ----------*/
#define MANUAL    0 // 手动模式(不启用PID)
#define AUTOMATIC 1 // 自动模式(启用PID自动调参)

#define DIRECT    0
#define REVERSE   1
/*---------- type define ----------*/

typedef struct {
    int32_t lastTime;
    double Input;
    double Output;
    double Setpoint;
    double ITerm; // ∑(ki*error) PS:为了实现动态调参把Ki放入积分项中
    double lastInput;
    double kp;
    double ki;
    double kd;
    int SampleTime; // 采样时间
    double outMin;
    double outMax;
    int dir;
    bool inAuto;
} PID_Controller;

typedef struct {
    int16_t Input;    // Q14
    int16_t Output;   // Q14
    int16_t Setpoint; // Q14
    int32_t Error;    // Q14
    int32_t PTerm;    // Q14
    int32_t ITerm;    // Q28 ∑(ki*error) PS:为了实现动态调参把Ki放入积分项中
    int16_t lastInput;
    int16_t kp;     // Q10
    int16_t ki;     // Q14
    int16_t outMin; // Q14
    int16_t outMax; // Q14
    bool inAuto;
} QPID_Controller;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
PID_Controller *pid_new(double kp, double ki, double kd, int SampleTime, double outMin, double outMax);
void pid_delete(PID_Controller *pid);
void pid_compute(PID_Controller *pid);
void pid_compute_raw(PID_Controller *pid, double abs_band);
void pid_set_tunings(PID_Controller *pid, double Kp, double Ki, double Kd);
void pid_set_sampletime(PID_Controller *pid, int NewSampleTime);
void pid_set_output_limits(PID_Controller *pid, double Min, double Max);
void pid_set_mode(PID_Controller *pid, int Mode);

// qpid
QPID_Controller *qpid_new(int16_t kp, int16_t ki, int16_t outMin, int16_t outMax);
void qpid_compute_raw(QPID_Controller *pid);
void qpid_set_mode(QPID_Controller *pid, int Mode);

/*---------- end of file ----------*/
#endif
