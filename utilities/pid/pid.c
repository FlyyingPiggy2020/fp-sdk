/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pid.c
 * @Author       : lxf
 * @Date         : 2024-12-02 08:21:53
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-26 16:45:05
 * @Brief        :
 * 移植自 Arduino PID Library
 * 
 * 如果在while(1)中调用，需要用户自行实现一个bsp_CheckRunTime的函数提供时基。
 * 下面给出一个bsp_CheckRunTime的例子，注意你需要处理溢出的问题：
 *
 * 或者在时间片中调用pid_compute_raw这个函数。PS:调用pid_compute_raw这个API的时候 SampleTime 无效。
 * 也就是说你无法动态修改修改PID环路时间
 */

// volatile int32_t g_iRunTime = 0;

// void SysTick_ISR(void)
// {
// 	/* anything else */
//     /* 全局运行时间每1ms增1 */
//     g_iRunTime++;
//     if (g_iRunTime == 0x7FFFFFFF) /* 这个变量是 int32_t 类型，最大数为 0x7FFFFFFF */
//     {
//         g_iRunTime = 0;
//     }

// }

// int32_t bsp_CheckRunTime(int32_t _LastTime)
// {
//     int32_t now_time;
//     int32_t time_diff;

//     DISABLE_INT(); /* 关中断 */

//     now_time = g_iRunTime; /* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

//     ENABLE_INT(); /* 开中断 */

//     if (now_time >= _LastTime) {
//         time_diff = now_time - _LastTime;
//     } else {
//         time_diff = 0x7FFFFFFF - _LastTime + now_time;
//     }

//     return time_diff;
// }

/*---------- includes ----------*/
#include "pid.h"
#include "bsp.h"
#include "heap.h"
#include <math.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

PID_Controller *pid_new(double kp, double ki, double kd, int SampleTime, double outMin, double outMax)
{
    PID_Controller *pid = (PID_Controller *)malloc(sizeof(PID_Controller)); // 动态分配内存
    if (pid == NULL) {
        return NULL; // 内存分配失败
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->SampleTime = SampleTime;
    pid->outMin = outMin;
    pid->outMax = outMax;
    pid->lastTime = 0;
    pid->Input = 0;
    pid->Output = 0;
    pid->Setpoint = 0;
    pid->ITerm = 0;
    pid->lastInput = 0;
    pid->dir = DIRECT;
    pid->inAuto = false;

    return pid;
}

void pid_delete(PID_Controller *pid)
{
    free(pid);
    pid = NULL;
}

void pid_compute(PID_Controller *pid)
{
    if (pid == NULL) {
        return;
    }
    if (!pid->inAuto) {
        return;
    }

    if (bsp_CheckRunTime(pid->lastTime) >= pid->SampleTime) {
        pid->lastTime = bsp_GetRunTime();
        double error = pid->Setpoint - pid->Input;
        pid->ITerm += (pid->ki * error);

        if (pid->ITerm > pid->outMax) {
            pid->ITerm = pid->outMax;
        } else if (pid->ITerm < pid->outMin) {
            pid->ITerm = pid->outMin;
        }
        double dInput = (pid->Input - pid->lastInput);
        pid->Output = pid->kp * error + pid->ITerm + pid->kd * (-dInput);
        if (pid->Output > pid->outMax) {
            pid->Output = pid->outMax;
        } else if (pid->Output < pid->outMin) {
            pid->Output = pid->outMin;
        }
        pid->lastInput = pid->Input;
    }
}

void pid_compute_raw(PID_Controller *pid, double abs_band)
{
    if (pid == NULL) {
        return;
    }
    if (!pid->inAuto) {
        return;
    }

    double error = pid->Setpoint - pid->Input;
    pid->ITerm += (pid->ki * error);

    if (pid->ITerm > pid->outMax) {
        pid->ITerm = pid->outMax;
    } else if (pid->ITerm < pid->outMin) {
        pid->ITerm = pid->outMin;
    }
    double dInput = (pid->Input - pid->lastInput);
    pid->Output = pid->kp * error + pid->ITerm + pid->kd * (-dInput);
    if (pid->Output > pid->outMax) {
        pid->Output = pid->outMax;
    } else if (pid->Output < pid->outMin) {
        pid->Output = pid->outMin;
    }
    pid->lastInput = pid->Input;
}

/**
 * @brief 动态调整PID参数
 * @param {double} Kp 调整后的Kp
 * @param {double} Ki 调整后的Ki
 * @param {double} Kd 调整后的Kd
 * @return {*}
 */
void pid_set_tunings(PID_Controller *pid, double Kp, double Ki, double Kd)
{
    if (Kp < 0 || Ki < 0 || Kd < 0) {
        return;
    }
    double SampleTimeInSec = ((double)pid->SampleTime) / 1000;
    pid->kp = Kp;
    pid->ki = Ki * SampleTimeInSec;
    pid->kd = Kd / SampleTimeInSec;
    if (pid->dir == REVERSE) {
        Kp = (0 - Kp);
        Ki = (0 - Ki);
        Kd = (0 - Kd);
    }
}

/**
 * @brief 修改PID环路时间
 * @param {int} NewSampleTime
 * @return {*}
 */
void pid_set_sampletime(PID_Controller *pid, int NewSampleTime)
{
    if (NewSampleTime > 0) {
        double ratio = (double)NewSampleTime / (double)pid->SampleTime;
        pid->ki *= ratio;
        pid->kd /= ratio;
        pid->SampleTime = (unsigned long)NewSampleTime;
    }
}

/**
 * @brief 设置钳位
 * @param {double} Min
 * @param {double} Max
 * @return {*}
 */
void pid_set_output_limits(PID_Controller *pid, double Min, double Max)
{
    if (Min > Max) {
        return;
    }
    pid->outMax = Max;
    pid->outMin = Min;
    if (pid->Output > pid->outMax) {
        pid->Output = pid->outMax;
    } else if (pid->Output < pid->outMin) {
        pid->Output = pid->outMin;
    }
    if (pid->ITerm > pid->outMax) {
        pid->ITerm = pid->outMax;
    } else if (pid->ITerm < pid->outMin) {
        pid->ITerm = pid->outMin;
    }
}
/**
 * @brief 初始化PID
 * @param {PID_Controller} *pid
 * @return {*}
 */
static void Initialize(PID_Controller *pid)
{
    pid->lastInput = pid->Input;
    pid->ITerm = pid->Output;
    if (pid->ITerm > pid->outMax) {
        pid->ITerm = pid->outMax;
    } else if (pid->ITerm < pid->outMin) {
        pid->ITerm = pid->outMin;
    }
}
/**
 * @brief PID开关
 * @param {PID_Controller} *pid
 * @param {int} Mode
 * @return {*}
 */
void pid_set_mode(PID_Controller *pid, int Mode)
{
    bool newAuto = (Mode == AUTOMATIC);
    if (newAuto && !pid->inAuto) {
        Initialize(pid);
    }
    pid->inAuto = newAuto;
}

void pid_set_direction(PID_Controller *pid, int Direction)
{
    pid->dir = Direction;
}
/*---------- end of file ----------*/
