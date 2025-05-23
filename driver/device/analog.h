/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : analog.h
 * @Author       : lxf
 * @Date         : 2025-05-23 10:08:46
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-23 10:09:03
 * @Brief        : 
 */


#ifndef __ANALOG_H__
#define __ANALOG_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/
#define IOCTL_ANALOG_ENABLE                             (IOCTL_USER_START + 0x00)
#define IOCTL_ANALOG_DISABLE                            (IOCTL_USER_START + 0x01)
#define IOCTL_ANALOG_GET                                (IOCTL_USER_START + 0x02)
#define IOCTL_ANALOG_SET_IRQ_HANDLER                    (IOCTL_USER_START + 0x03)
/*---------- type define ----------*/
typedef struct {
    uint32_t number_of_channels;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*enable)(bool ctrl);
        uint32_t (*get)(uint32_t channel);
        int32_t (*irq_handler)(uint32_t irq, void *args, uint32_t length);
    } ops;
} analog_describe_t;

union analog_ioctl_param {
    struct {
        uint32_t channel;
        uint32_t data;
    } get;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__ANALOG_H__



