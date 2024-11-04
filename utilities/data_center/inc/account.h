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

#include "pingpong.h"
#include "fp_soft_timer.h"
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

typedef struct data_center data_center_t;
typedef struct account account_t;

typedef struct {
    account_event_code_t event;
    account_t *tran;
    account_t *recv;
    void *data;
    unsigned int len;
} account_event_param_t;

typedef int (*event_callback_t)(account_t *account, account_event_param_t *param);

typedef struct account {
    const char *id;
    data_center_t *center;
    unsigned int buff_size;
    void *user_data;

    struct list_head followers_list; // 关注者列表
    struct list_head fans_list;      // 粉丝列表
    struct {
        event_callback_t event_call_back;
        fp_timer_t *timer;
        pingpong_buffer_t buffer_manager;
        unsigned int buffer_size;
    } priv;
} account_t;

typedef struct account_node {
    account_t *account;
    struct list_head node;
} account_node_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
account_t *account_init(const char *id, data_center_t *center, unsigned int buffer_size, void *user_data);
void account_deinit(account_t *account);
account_t *account_subscribe(account_t *account, const char *pub_id);
bool account_unsubscribe(account_t *account, const char *pub_id);
bool account_commit(account_t *account, const void *data, unsigned int size);
int account_pubilsh(account_t *account);
int account_pull_from_account(account_t *account, account_t *pub, void *data, unsigned int size);
int account_pull_from_id(account_t *account, const char *pub_id, void *data, unsigned int size);
int account_notify_from_account(account_t *account, account_t *pub, void *data, unsigned int size);
int account_notify_from_id(account_t *account, const char *pub_id, void *data, unsigned int size);
void account_set_event_callback(account_t *account, event_callback_t cb);
void account_set_timer_period(account_t *account, uint32_t period);
void account_set_timer_enable(account_t *account, bool en);
/*---------- end of file ----------*/
#endif // !__ACCOUNT_H__
