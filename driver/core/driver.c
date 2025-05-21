/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : driver.c
 * @Author       : lxf
 * @Date         : 2024-12-05 10:49:38
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-10 16:29:03
 * @Brief        : 从岑老板的代码中抄袭一部分
 */

/*---------- includes ----------*/
#include "driver.h"
#include "device.h"
#include "string.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/

/*---------- variable ----------*/
/*---------- function ----------*/

int32_t driver_search_device(void)
{
#if defined(__linux__) || defined(_WIN32)
    extern driver_t __start_drv_defined;
    extern driver_t __stop_drv_defined;
    extern device_t __start_dev_defined;
    extern device_t __stop_dev_defined;
    driver_t *__begin_drv = &__start_drv_defined;
    driver_t *__end_drv = &__stop_drv_defined;
    device_t *__begin_dev = &__start_dev_defined;
    device_t *__end_dev = &__stop_dev_defined;
#elif defined(__ARMCC_VERSION)
    extern driver_t Image$$DRV_REGION$$Base[];
    extern driver_t Image$$DRV_REGION$$Limit[];
    extern device_t Image$$DEV_REGION$$Base[];
    extern device_t Image$$DEV_REGION$$Limit[];
    driver_t *__begin_drv = Image$$DRV_REGION$$Base;
    driver_t *__end_drv = Image$$DRV_REGION$$Limit;
    device_t *__begin_dev = Image$$DEV_REGION$$Base;
    device_t *__end_dev = Image$$DEV_REGION$$Limit;
#else
    extern driver_t _sdrv;
    extern driver_t _edrv;
    extern device_t _sdev;
    extern device_t _edev;
    driver_t *__begin_drv = &_sdrv;
    driver_t *__end_drv = &_edrv;
    device_t *__begin_dev = &_sdev;
    device_t *__end_dev = &_edev;
#endif
    driver_t *drv = NULL;
    device_t *dev = NULL;

    for (dev = __begin_dev; dev < __end_dev; ++dev) {
        for (drv = __begin_drv; drv < __end_drv; ++drv) {
            if (!strncmp(dev->drv_name, drv->drv_name, sizeof(drv->drv_name))) {
                dev->pdrv = drv;
            }
        }
    }

    return 0;
}
/*---------- end of file ----------*/
