/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : driver.h
 * @Author       : lxf
 * @Date         : 2024-12-05 10:49:38
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-13 10:19:36
 * @Brief        : 设备IO驱动框架
 */

#ifndef __DRIVER_H__
#define __DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <define.h>
/*---------- macro ----------*/
#undef _DEV_SECTION_PREFIX
#if defined(__linux__) || defined(_WIN32)
#define _DEV_SECTION_PREFIX
#else
#define _DEV_SECTION_PREFIX "."
#endif

#define DRIVER_DEFINED(name, open, close, write, read, ioctl, irq_handler)                   \
    driver_t driver_##name __attribute__((used, section(_DEV_SECTION_PREFIX "drv_defined"))) \
    __attribute__((aligned(4))) = { #name, open, close, write, read, ioctl, irq_handler }

/*---------- type define ----------*/
typedef struct st_driver driver_t; // 前项声明

typedef struct st_driver {
    char drv_name[10];
    fp_err_t (*open)(driver_t **drv);
    void (*close)(driver_t **drv);
    fp_err_t (*write)(driver_t **drv, void *buf, uint32_t addition, uint32_t len);
    fp_err_t (*read)(driver_t **drv, void *buf, uint32_t addition, uint32_t len);
    fp_err_t (*ioctl)(driver_t **drv, uint32_t cmd, void *args);
    fp_err_t (*irq_handler)(driver_t **drv, uint32_t irq_handler, void *args, uint32_t len);
} driver_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
fp_err_t driver_search_device(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
