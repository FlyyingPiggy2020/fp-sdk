/*MIT License

Copyright (c) 2023 Lu Xianfan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : device_manager.c
 * @Author       : lxf
 * @Date         : 2023-12-14 10:52:42
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-20 13:12:04
 * @Brief        : 设备管理
 */

/*---------- includes ----------*/

#undef LOG_TAG
#define LOG_TAG "devmngr"
#undef LOG_OUTPUT_LVL
#define LOG_OUTPUT_LVL LOG_LVL_VERBOSE

#include "inc/device_manager.h"
#include "../../utilities/log/inc/log.h"
#include "../../utilities/export/inc/export.h"

#if (FP_USE_SHELL == 1)
#include "../../utilities/shell/inc/shell.h"
#endif

#include "string.h"

/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/

LIST_HEAD(device_list);
/*---------- function prototype ----------*/

/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief 根据名称查找设备
 * @param {char} *name
 * @return {*}没有找到则返回NULL
 */
device_t *device_find_name(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    device_t *dev = NULL;
    list_for_each_entry(dev, device_t, &device_list, list)
    {
        if (strncmp(dev->name, name, DEVICE_NAME_MAX) == 0) {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 设备注册
 * @param {device_t} device
 * @param {char} *name
 * @param {int} flag
 * @return {*}
 */
fp_err_t device_register(device_t *dev, const char *name, uint16_t flags)
{
    if (dev == NULL) {
        return FP_ERROR;
    }

    if (device_find_name(name) != NULL) {
        return FP_ERROR;
    }

    dev->flag = flags;
    dev->count = 0;
    dev->open_flag = 0;

    strncpy(dev->name, name, DEVICE_NAME_MAX);
    list_add(&dev->list, &device_list);
    return FP_EOK;
}

/**
 * @brief 设备解除注册，它会解除静态绑定，并不会释放内存
 * @param {device_t} dev
 * @return {*}
 */
fp_err_t device_unregister(device_t *dev)
{
    assert(dev);
    list_del(&dev->list);
    return FP_ERROR;
}

fp_err_t device_init(device_t *dev)
{
    fp_err_t result = FP_EOK;

    assert(dev != NULL);

    if (dev->ops->init != NULL) {
        if (!(dev->flag & DEVICE_FLAG_ACTIVATED)) {
            result = dev->ops->init(dev);
            if (result != FP_EOK) {
                log_e("To initialize device:%s failed.The error code is %d", dev->name, result);
            }
            else {
                dev->flag |= DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return result;
}
/**
 * @brief 打开设备
 * @param {device_t} *dev
 * @return {*}
 */
fp_err_t device_open(device_t *dev, uint16_t oflag)
{
    fp_err_t result = FP_EOK;

    assert(dev);
    assert(dev->ops);

    /* 如果设备未初始化，则先初始化 */
    if (!(dev->flag & DEVICE_FLAG_ACTIVATED)) {
        if (dev->ops->init != NULL) {
            result = dev->ops->init(dev);
            if (result != FP_EOK) {
                log_e("To initialize device :%s failed. The error code is %d", dev->name, result);
                return result;
            }
        }
        dev->flag |= DEVICE_FLAG_ACTIVATED;
    }

    /* 如果是独立设备并且已经打开，不允许重复打开 */
    if ((dev->flag & DEVICE_FLAG_STANDALONE) && (dev->open_flag & DEVICE_OFLAG_OPEN)) {
        return -FP_EBUSY;
    }

    /* 如果设备未打开或者打开标志与设备本身的标志符，则调用打开函数 */
    if (!(dev->open_flag & DEVICE_OFLAG_OPEN) || ((dev->open_flag & DEVICE_OFLAG_MASK) != (oflag & DEVICE_OFLAG_MASK))) {
        if (dev->ops->open != NULL) {
            result = dev->ops->open(dev);
        }
        else {
			log_e("To open device :%s failed. can't find open funciton", dev->name);
            dev->open_flag = (oflag & DEVICE_OFLAG_MASK);
        }
    }
	if (result != FP_EOK) {
		log_e("To open device :%s failed. The error code is %d", dev->name, result);
	}
    /* 设置打开标志 */
    if (result == FP_EOK || result == -FP_ENOSYS) {
        dev->open_flag |= DEVICE_OFLAG_OPEN;

        dev->count++;
        /* 如果断言函数被执行，是因为count值溢出了 */
        assert(dev->count != 0);
    }
    return result;
}

/**
 * @brief 关闭设备
 * @param {device_t} *dev
 * @return {*}
 */
fp_err_t device_close(device_t *dev)
{
    fp_err_t result = FP_EOK;
    assert(dev != NULL);

    if (dev->count == 0) {
        return -FP_ERROR;
    }

    dev->count--;

    if (dev->count != 0) {
        return FP_EOK;
    }

    if (dev->ops->close != NULL) {
        result = dev->ops->close(dev);
    }

    if (result == FP_EOK || result == -FP_ENOSYS) {
        dev->open_flag = DEVICE_OFLAG_CLOSE;
    }
    return result;
}

/**
 * @brief 从设备读取一些数据
 * @param {device_t} *dev
 * @param {int} pos 开始读的时候的位置
 * @param {void} *buffer 读数据的缓存
 * @param {int} size 读数据的大小
 * @return {*} 如果读取成功返回实际读取的size，否则返回0
 */
fp_size_t device_read(device_t *dev, int pos, void *buffer, int size)
{
    assert(dev != NULL);

    if (dev->count == 0) {
        return 0;
    }
    if (dev->ops->read != NULL) {
        return dev->ops->read(dev, pos, buffer, size);
    }
    return 0;
}

/**
 * @brief 向设备写入一些数据
 * @param {device_t} *dev
 * @param {int} pos
 * @param {void} *buffer
 * @param {int} size
 * @return {*} 如果写入成功返回实际写入的size，否则返回0
 */
fp_size_t device_write(device_t *dev, int pos, const void *buffer, int size)
{
    assert(dev != NULL);

    if (dev->count == 0) {
        return 0;
    }
    if (dev->ops->write != NULL) {
        return dev->ops->write(dev, pos, buffer, size);
    }

    return 0;
}

/**
 * @brief 设备控制
 * @param {device_t} *dev
 * @param {int} cmd
 * @param {void} *arg
 * @return {*}
 */
fp_err_t device_control(device_t *dev, int cmd, void *arg)
{
    assert(dev != 0);

    if (dev->ops->ioctl != NULL) {
        return dev->ops->ioctl(dev, cmd, arg);
    }
    return -FP_ENOSYS;
}

/**
 * @brief 设置接收中断
 * @param {device_t} *dev
 * @return {*}
 */
fp_err_t device_set_rx_indicate(device_t *dev, fp_err_t (*rx_ind)(device_t *dev, fp_size_t size))
{
    assert(dev != NULL);
    dev->rx_indicate = rx_ind;
    return FP_EOK;
}

fp_err_t device_set_tx_complete(device_t *dev, fp_err_t (*tx_done)(device_t *dev, void *buffer))
{
    assert(dev != NULL);
    dev->tx_complete = tx_done;
    return FP_EOK;
}

#if (FP_USE_SHELL == 1)
/**
 * @brief 查看已注册设备列表
 * @return {*}
 */
int device_list_display(Shell *shell, uint8_t argc, char *argv[])
{
    device_t *dev = NULL;
    list_for_each_entry(dev, device_t, &device_list, list)
    {
        shell->shell_write("\r\n",2);
        shell->shell_write(dev->name, DEVICE_NAME_MAX);
    }
    shell->shell_write("\r\n",2);
    return 0;
}
SHELL_EXPORT_CMD(device_list, device_list_display);
#endif
/*---------- end of file ----------*/
