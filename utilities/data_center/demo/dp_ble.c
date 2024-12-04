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
 * @FilePath     : dp_ble.c
 * @Author       : lxf
 * @Date         : 2024-07-30 15:30:20
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-30 15:30:58
 * @Brief        :
 */

/*---------- includes ----------*/
#include "data_proc.h"
/*---------- macro ----------*/

#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))
/*---------- type define ----------*/
#define BLE_NAME_MAX_LENGTH 12
/*---------- variable prototype ----------*/
typedef struct {
    uint32_t type;
    void (*cb)(account_event_param_t *);
} protocol_callback_t;

static char ble_name[BLE_NAME_MAX_LENGTH] = "ARMFLY";
/*---------- function prototype ----------*/
static void _on_event_sub_pull(account_event_param_t *param);
static void _on_event_publish(account_event_param_t *param);
static void _on_event_nofity(account_event_param_t *param);
/*---------- variable ----------*/
static protocol_callback_t act_evt_cbs[] = {
    { ACCOUNT_EVENT_SUB_PULL, _on_event_sub_pull },
    { ACCOUNT_EVENT_PUB_PUBLISH, _on_event_publish },
    { ACCOUNT_EVENT_NOTIFY, _on_event_nofity },
};
/*---------- function ----------*/

static void _on_event_sub_pull(account_event_param_t *param)
{
    dp_ble_info_t *info = (dp_ble_info_t *)param->data;
    info->name = ble_name;
    return;
}

static void _on_event_publish(account_event_param_t *param)
{
    return;
}

static void _on_event_nofity(account_event_param_t *param)
{
    dp_ble_info_t *info = (dp_ble_info_t *)param->data;
    if (info->cmd == DP_BLE_START) {
        printf("TODO:START BLE\n");
    }
    if (info->cmd == DP_BLE_STOP) {
        printf("TODO:STOP BLE\n");
    }
    return;
}
static int on_event(account_t *account, account_event_param_t *param)
{
    int err = 0;
    account_event_code_t evt = param->event;
    void (*cb)(account_event_param_t *) = NULL;

    for (int32_t i = 0; i <= ARRAY_SIZE(act_evt_cbs); i++) {
        if (evt == act_evt_cbs[i].type) {
            cb = act_evt_cbs[i].cb;
            break;
        }
    }
    if (cb) {
        cb(param);
    }

    return err;
}

DATA_PROC_INIT_DEF(ble)
{
    account_subscribe(account, "ble");
    account_set_event_callback(account, on_event);
}
/*---------- end of file ----------*/
