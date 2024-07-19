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
 * @FilePath     : account.h
 * @Author       : lxf
 * @Date         : 2024-07-19 14:29:52
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-19 14:32:17
 * @Brief        :
 */

#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__
/*---------- includes ----------*/

#include "../pingpong/inc/pingpong.h"
#include "../soft_timer/fp_soft_timer.h"
#include "data_center.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
typedef enum {
    ACCOUNT_EVENT_NONE,
    ACCOUNT_EVENT_PUB_PUBLISH,
    ACCOUNT_EVENT_SUB_PULL,
    ACCOUNT_EVENT_NOTIFY,
    ACCOUNT_EVENT_TIMER,
    ACCOUNT_EVENT_LAST,
} account_event_code_t;

typedef enum {
    ACCOUNT_RES_CODE_OK = 0,
    ACCOUNT_RES_UNKNOW = -1,
    ACCOUNT_RES_SIZE_MISMATCH = -2,
    ACCOUNT_RES_UNSUPPORTED_REQUEST = -3,
    ACCOUNT_RES_CODE_NO_CALLBACK = -4,
    ACCOUNT_RES_CODE_NO_CACHE = -5,
    ACCOUNT_RES_NO_COMMITED = -6,
    ACCOUNT_RES_NOT_FOUND = -7,
    ACCOUNT_RES_PARAM_ERROR = -8,
} account_res_code_t;

typedef int (*event_callback_t)(account_t *account, account_event_param_t *param);
typedef struct account {
    const char *id;
    data_center_t *center;
    unsigned int buff_size;
    void *user_data;

    struct list_head account_pool_node;
    struct list_head publishers;
    struct list_head subscribers;

    struct {
        event_callback_t event_call_back;
        fp_timer_t *timer;
        pingpong_buffer_t buffer_manager;
        unsigned int buffer_size; 
    }priv;
    
}account_t;

typedef struct {
    account_event_code_t event;
    account_t *tran;
    account_t *recv;
    void *data;
    unsigned int len;
} account_event_param_t;


/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif // !__ACCOUNT_H__
