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
/*---------- macro ----------*/

#define DEVICE_NAME_MAX 10 /* 最大名称字节数 */
/*---------- type define ----------*/

typedef struct
{
    int (*open)(void *dev);
    int (*close)(void *dev);
    int (*read)(void *dev, void *buffer, int size);
    int (*write)(void *dev, const void *buffer, int size);
    int (*ioctl)(void *dev, int request, void *arg);
} device_operations;

typedef struct
{
    const char name[DEVICE_NAME_MAX];
    void *private_data;     // 设备私有的数据
    uint16_t count;         // 操作次数(如果支持OS，则支持多次open同一个设备，只有最后一次close的时候才会释放资源，这个标志用于计数)
    device_operations *ops; // 指向设备操作的指针
    struct list_head list;  // 链表
} device_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

int32_t device_register(device_t *dev, const char *name);
/*---------- end of file ----------*/
#endif
