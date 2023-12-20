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
 * @FilePath     : driver.h
 * @Author       : lxf
 * @Date         : 2023-12-14 10:52:47
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-14 10:52:50
 * @Brief        : 设备驱动框架
 */

#ifndef __DRIVER_H__
#define __DRIVER_H__
/*---------- includes ----------*/
/*---------- macro ----------*/
/*---------- type define ----------*/

typedef struct
{
    int (*device_open)(void *device);
    int (*device_close)(void *device);
    int (*device_read)(void *device, void *buffer, int size);
    int (*device_write)(void *device, const void *buffer, int size);
    int (*device_ioctl)(void *device, int request, void *arg);
} device_operations;

typedef struct
{
    const char *name;
    void *private_data;     // 设备特定的数据
    device_operations *ops; // 指向设备操作的指针
} device;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
