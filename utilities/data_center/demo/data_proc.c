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
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : data_proc.c
 * @Author       : lxf
 * @Date         : 2024-07-30 15:01:12
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-30 15:08:37
 * @Brief        :
 */

/*---------- includes ----------*/
#include "data_proc.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/

static data_center_t *data_center = NULL;

#define DATA_PROC_DEF(name, buffer_size) account_t *act_##name
#include "dp_list.inc"
#undef DATA_PROC_DEF
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
void data_proc_init(void)
{
    data_center = data_center_init("center");
#define DATA_PROC_DEF(name, buffer_size) act_##name = account_init(#name, data_center, buffer_size, NULL);
#include "dp_list.inc"
#undef DATA_PROC_DEF

#define DATA_PROC_DEF(name, buffer_size)                          \
    do {                                                          \
        extern void _data_proc_##name##_init(account_t *account); \
        _data_proc_##name##_init(act_##name);                     \
    } while (0)
#include "dp_list.inc"
#undef DATA_PROC_DEF
}

data_center_t *data_proc_get_center(void)
{
    return data_center;
}
/*---------- end of file ----------*/
