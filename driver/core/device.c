/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : device.c
 * @Author       : lxf
 * @Date         : 2024-12-05 13:10:51
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-05 16:47:28
 * @Brief        : 从岑老板的代码中抄袭
 */

/*---------- includes ----------*/
#include "device.h"
#include "driver.h"
#include "drv_err.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
void *device_open(char *name)
{
#if defined(__linux__) || defined(_WIN32)
    extern device_t __start_dev_defined;
    extern device_t __stop_dev_defined;
    device_t *__begin_dev = &__start_dev_defined;
    device_t *__end_dev = &__stop_dev_defined;
#elif defined(__ARMCC_VERSION)
    extern device_t Image$$DEV_REGION$$Base[];
    extern device_t Image$$DEV_REGION$$Limit[];
    device_t *__begin_dev = Image$$DEV_REGION$$Base;
    device_t *__end_dev = Image$$DEV_REGION$$Limit;
#else
    extern device_t _sdev;
    extern device_t _edev;
    device_t *__begin_dev = &_sdev;
    device_t *__end_dev = &_edev;
#endif
    device_t *dev = NULL;
    int32_t result = DRV_ERR_OK;

    for (dev = __begin_dev; dev < __end_dev; ++dev) {
        if (!strncmp(name, dev->dev_name, sizeof(dev->dev_name))) {
            if (!__device_attrib_isstart(dev->attribute)) {
                if (dev->pdrv && ((driver_t *)dev->pdrv)->open) {
                    result = ((driver_t *)dev->pdrv)->open((driver_t **)(&dev->pdrv));
                }
                if (DRV_ERR_OK == result) {
                    __device_attrib_setstart(dev->attribute, DEVICE_ATTRIB_START);
                }
            }
            if (DRV_ERR_OK == result) {
                dev->count++;
            }
            break;
        }
    }

    return ((dev == __end_dev) || (DRV_ERR_OK != result)) ? NULL : dev;
}

void device_close(device_t *dev)
{
    do {
        if (!dev) {
            break;
        }
        if (dev->count) {
            dev->count--;
        }
        if (dev->count) {
            break;
        }
        if (__device_attrib_isstart(dev->attribute)) {
            if (dev->pdrv && ((driver_t *)dev->pdrv)->close) {
                ((driver_t *)dev->pdrv)->close((driver_t **)(&dev->pdrv));
            }
            __device_attrib_setstart(dev->attribute, DEVICE_ATTRIB_IDLE);
        }
    } while (0);
}

int32_t device_write(device_t *dev, void *buf, uint32_t addition, uint32_t len)
{
    int32_t retval = DRV_ERR_POINT_NONE;

    if (dev && dev->pdrv && ((driver_t *)dev->pdrv)->write) {
        retval = ((driver_t *)dev->pdrv)->write((driver_t **)(&dev->pdrv), buf, addition, len);
    }

    return retval;
}

int32_t device_read(device_t *dev, void *buf, uint32_t addition, uint32_t len)
{
    int32_t retval = DRV_ERR_POINT_NONE;

    if (dev && dev->pdrv && ((driver_t *)dev->pdrv)->read) {
        retval = ((driver_t *)dev->pdrv)->read((driver_t **)(&dev->pdrv), buf, addition, len);
    }

    return retval;
}

int32_t device_ioctl(device_t *dev, uint32_t cmd, void *args)
{
    int32_t retval = DRV_ERR_POINT_NONE;

    if (dev && dev->pdrv && ((driver_t *)dev->pdrv)->ioctl) {
        retval = ((driver_t *)dev->pdrv)->ioctl((driver_t **)(&dev->pdrv), cmd, args);
    }

    return retval;
}

int32_t device_irq_process(device_t *dev, uint32_t irq_handler, void *args, uint32_t len)
{
    int32_t retval = DRV_ERR_POINT_NONE;

    if (dev && dev->pdrv && ((driver_t *)dev->pdrv)->irq_handler) {
        retval = ((driver_t *)dev->pdrv)->irq_handler((driver_t **)(&dev->pdrv), irq_handler, args, len);
    }

    return retval;
}
/*---------- end of file ----------*/
