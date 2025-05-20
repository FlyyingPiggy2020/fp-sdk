/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : device.h
 * @Author       : lxf
 * @Date         : 2024-12-05 13:10:55
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-05 13:18:56
 * @Brief        :
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
/*---------- macro ----------*/

#undef _DEV_SECTION_PREFIX
#if defined(__linux__) || defined(_WIN32)
#define _DEV_SECTION_PREFIX
#else
#define _DEV_SECTION_PREFIX "."
#endif

#define DEVICE_DEFINED(dev_name, drv_name, desc)                                                 \
    device_t device_##dev_name __attribute__((used, section(_DEV_SECTION_PREFIX "dev_defined"))) \
    __attribute__((aligned(4))) = { #dev_name, #drv_name, 0, 0, NULL, desc }

#define IOCTL_USER_START                        (0X80000000)
#define IOCTL_DEVICE_POWER_ON                   (0x00001000)
#define IOCTL_DEVICE_POWER_OFF                  (0x00001001)

#define DEVICE_ATTRIB_IDLE                      (0x00)
#define DEVICE_ATTRIB_START                     (0x01)
#define DEVICE_ATTRIB_POWER_OFF                 (0x0 << 4U)
#define DEVICE_ATTRIB_POWER_ON                  (0x1 << 4U)

#define __device_attrib_ispower(attrib)         ((attrib & 0xF0) == DEVICE_ATTRIB_POWER_ON)
#define __device_attrib_isstart(attrib)         ((attrib & 0x0F) == DEVICE_ATTRIB_START)
#define __device_attrib_setpower(attrib, power) (attrib = (attrib & (~0xF0)) | power)
#define __device_attrib_setstart(attrib, start) (attrib = (attrib & (~0x0F)) | start)
/*---------- type define ----------*/

typedef struct {
    char dev_name[16];
    char drv_name[16];
    uint16_t attribute;
    uint32_t count;
    struct st_driver *pdrv;
    void *pdesc;
} device_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void *device_open(char *name);
extern void device_close(device_t *dev);
extern int32_t device_write(device_t *dev, void *buf, uint32_t addition, uint32_t len);
extern int32_t device_read(device_t *dev, void *buf, uint32_t addition, uint32_t len);
extern int32_t device_ioctl(device_t *dev, uint32_t cmd, void *args);
extern int32_t device_irq_process(device_t *dev, uint32_t irq_handler, void *args, uint32_t len);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_H */
