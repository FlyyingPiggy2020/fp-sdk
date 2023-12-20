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

#include "device_manager.h"
#include "log.h"
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
 * @return {*}-1注册失败;0注册成功
 */
int32_t device_register(device_t *dev, const char *name)
{
    if (dev == NULL) {
        return -1;
    }

    if (device_find_name(name) != NULL) {
        return -1;
    }

    strncpy(dev->name, name, DEVICE_NAME_MAX);
    list_add(&dev->list, &device_list);
}

/**
 * @brief 设备解除注册，它会解除静态绑定，并不会释放内存
 * @param {device_t} dev
 * @return {*}
 */
int32_t device_unregister(device_t *dev)
{
    assert(dev == NULL);
    list_del(&dev->list);
}

int32_t device_open(device_t *dev)
{
    assert(dev);

    if (dev->ops->open != NULL) {
        dev->ops->open(dev);
    }
}
/*---------- end of file ----------*/
