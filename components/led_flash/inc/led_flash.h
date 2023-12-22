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
 * @FilePath     : led_flash.h
 * @Author       : lxf
 * @Date         : 2023-12-14 09:41:06
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-14 10:18:32
 * @Brief        : led闪烁头文件
 */

#ifndef __LED_FLASH_H__
#define __LED_FLASH_H__
/*---------- includes ----------*/

#include "stdint.h"
/*---------- macro ----------*/

#define FP_LED_ON  1
#define FP_LED_OFF 0
/*---------- type define ----------*/
typedef struct led_attribute
{

} led_attribute_t;

typedef struct led_flash_handle
{
    led_attribute_t led; /* led */
    int8_t frequency;    /* 闪烁频率 */
    int16_t count;       /* 闪烁次数 */
    int8_t priority;     /* 优先级 */
    void (*ioctl)(void);
} led_flash_handle_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
