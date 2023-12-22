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
 * @FilePath     : device_manager.h
 * @Author       : lxf
 * @Date         : 2023-12-14 10:52:47
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-20 13:12:17
 * @Brief        : 设备管理
 */

#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__
/*---------- includes ----------*/

#include "clists.h"
#include "fp_def.h"

/*---------- macro ----------*/

#define DEVICE_NAME_MAX        10 /* 最大名称字节数 */

#define DEVICE_FLAG_DEACTIVETE 0x000 /* 未激活 */
/*---------- type define ----------*/

typedef struct
{
    fp_err_t (*init)(void *dev);
    fp_err_t (*open)(void *dev);
    fp_err_t (*close)(void *dev);
    fp_size_t (*read)(void *dev, int pos, void *buffer, int size);
    fp_size_t (*write)(void *dev, int pos, const void *buffer, int size);
    fp_err_t (*ioctl)(void *dev, int cmd, void *arg);
} device_operations;

typedef struct device device_t; // 前向声明

struct device
{
    char name[DEVICE_NAME_MAX];
    void *private_data; // 设备私有的数据
    uint16_t count;     // 操作次数(如果支持OS，则支持多次open同一个设备，只有最后一次close的时候才会释放资源，这个标志用于计数)
    uint16_t flag;      // 标志
    uint16_t open_flag; // 打开标志
    fp_err_t (*rx_indicate)(device_t *dev, fp_size_t size); // 接收中断
    fp_err_t (*tx_complete)(device_t *dev, void *buffer);   // 发送完成
    device_operations *ops;                                 // 指向设备操作的指针
    struct list_head list;                                  // 链表
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

device_t *device_find_name(const char *name);
fp_err_t device_register(device_t *dev, const char *name, uint16_t flags);
fp_err_t device_open(device_t *dev, uint16_t oflag);
fp_err_t device_close(device_t *dev);
fp_size_t device_read(device_t *dev, int pos, void *buffer, int size);
fp_size_t device_write(device_t *dev, int pos, const void *buffer, int size);
fp_err_t device_control(device_t *dev, int cmd, void *arg);
fp_err_t device_set_rx_indicate(device_t *dev, fp_err_t (*rx_ind)(device_t *dev, fp_size_t size));
fp_err_t device_set_tx_complete(device_t *dev, fp_err_t (*tx_done)(device_t *dev, void *buffer));
/*---------- end of file ----------*/
#endif
