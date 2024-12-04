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
 * @FilePath     : data_center.h
 * @Author       : lxf
 * @Date         : 2024-07-19 14:29:26
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-19 15:39:59
 * @Brief        :
 */

#ifndef __DATA_CENTER_H__
#define __DATA_CENTER_H__
/*---------- includes ----------*/
#include "data_center.h"
#include "account.h"
/*---------- macro ----------*/
#if defined(CONF_BOARD_NAME_BL60X)
#include "FreeRTOS.h"
#define __malloc pvPortMalloc
#define __free   vPortFree
#else
#include "tlsf.h"
#define __malloc tlsf_malloc
#define __free   tlsf_free
#endif
/*---------- type define ----------*/
typedef struct data_center data_center_t;
typedef struct account account_t;

typedef struct data_center {
    const char *name;
    account_t *account_main;
    struct list_head account_pool;
} data_center_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

data_center_t *data_center_init(const char *name);
void data_center_deinit(data_center_t *center);
bool datacenter_add_account(data_center_t *center, account_t *account);
bool datacenter_remove_account(data_center_t *center, account_t *account);
/*---------- end of file ----------*/
#endif // !__DATA_CENTER_H__
