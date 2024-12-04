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
 * @FilePath     : account.c
 * @Author       : lxf
 * @Date         : 2024-07-19 14:29:46
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-19 15:54:26
 * @Brief        :
 */

/*---------- includes ----------*/
#if FP_LOG_TRACE_DATA_CENTER
#undef LOG_TAG
#define LOG_TAG "DATA_CENTER"
#include "log_port.h"
#define DATA_CENTER_TRACE(...) log_w(__VA_ARGS__)
#else
#define DATA_CENTER_TRACE(...)
#endif

#include "fp_soft_timer.h"
#include "account.h"
#include "string.h"
/*---------- macro ----------*/

#define ACCOUNT_MAX_ID_LENGTH 16

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern bool _datacenter_remove(struct list_head *pool, account_t *account);
extern account_t *_search_account(data_center_t *center, const char *id);
extern account_t *_datacenter_find(struct list_head *pool, const char *id);
/*---------- variable ----------*/
/*---------- function ----------*/

static bool inline __match_by_name(const char *s1, const char *s2)
{
    return (strcmp(s1, s2) == 0);
}

account_t *account_init(const char *id, data_center_t *center, unsigned int buffer_size, void *user_data)
{
    account_t *new = __malloc(sizeof(account_t));
    bool error = false;
    do {
        if (new == NULL) {
            DATA_CENTER_TRACE("malloc new account failed.");
            break;
        }

        error = true;
        memset(&new->priv, 0, sizeof(new->priv));
        new->id = id;
        new->center = center;
        new->user_data = user_data;
        INIT_LIST_HEAD(&new->fans_list);
        INIT_LIST_HEAD(&new->followers_list);
        if (buffer_size != 0) {
            unsigned char *buffer = __malloc(buffer_size * 2);
            if (buffer == NULL) {
                DATA_CENTER_TRACE("account[%s] buffer malloc failed", id);
                break;
            }
            memset(buffer, 0, buffer_size * 2);
            unsigned char *buf0 = buffer;
            unsigned char *buf1 = buffer + buffer_size;
            pingpong_buffer_init(&new->priv.buffer_manager, buf0, buf1);
            new->priv.buffer_size = buffer_size;
            DATA_CENTER_TRACE("account[%s] cached %dx2 bytes", id, buffer_size);
        }
        if (datacenter_add_account(center, new) == false) {
            DATA_CENTER_TRACE("account[%s] add to center[%s] failed", id, center->name);
            break;
        }
        error = false;
    } while (0);

    if (error == true) {
        __free(new);
        new = NULL;
    }
    return new;
}

void account_deinit(account_t *account)
{
    DATA_CENTER_TRACE("account[%s] deleting...", account->id);

    account_node_t *p, *n;
    /* release cache */
    if (account->priv.buffer_size) {
        __free(account->priv.buffer_manager.buffer[0]);
    }
    /* delete timer */
    if (account->priv.timer) {
        fp_timer_del(account->priv.timer);
        DATA_CENTER_TRACE("account[%s] task deleted", account->id);
    }
    /* let fans unfollow */
    list_for_each_entry_safe(p, n, account_node_t, &account->fans_list, node)
    {
        account_unsubscribe(p->account, account->id);
        DATA_CENTER_TRACE("account[%s] unfollowed %s", p->account->id, account->id);
    }
    /* ask the publisher to delete this fans */
    list_for_each_entry_safe(p, n, account_node_t, &account->followers_list, node)
    {
        _datacenter_remove(&p->account->fans_list, account);
        DATA_CENTER_TRACE("account[%s] unfollowed %s", account->id, p->account->id);
    }
    /* let the data center delete the account */
    datacenter_remove_account(account->center, account);
    DATA_CENTER_TRACE("account[%s] deleted", account->id);
    __free(account);
}

/**
 * @brief subscribe to publisher[订阅发布者]
 * @param {account_t} *account
 * @param {char} *pub_id : publisher id
 * @return {*} pointer to the publisher account
 */
account_t *account_subscribe(account_t *account, const char *pub_id)
{
    account_t *publisher = NULL;
    account_node_t *pub = NULL, *sub = NULL;
    uint8_t error_flag = 0;

    do {
        if (account == NULL || pub_id == NULL) {
            break;
        }

        if (__match_by_name(account->id, pub_id) == true) {
            DATA_CENTER_TRACE("account[%s] can't subscribe to itself", account->id);
            break;
        }
        pub = __malloc(sizeof(account_node_t));
        if (pub == NULL) {
            DATA_CENTER_TRACE("malloc pub node[%s] failed", pub_id);
            break;
        }
        error_flag = 1;
        sub = __malloc(sizeof(account_node_t));
        if (sub == NULL) {
            DATA_CENTER_TRACE("malloc sub node[%s] failed", account->id);
            break;
        }
        error_flag = 2;
        publisher = _datacenter_find(&account->followers_list, pub_id);
        if (publisher != NULL) {
            DATA_CENTER_TRACE("account [%s] multi subscribe pub[%s]", account->id, pub_id);
            break;
        }
        publisher = _search_account(account->center, pub_id);
        if (publisher == NULL) {
            DATA_CENTER_TRACE("account[%s] was not found", pub_id);
            break;
        }
        /* add the publisher to the subscription list */
        memset(pub, 0, sizeof(account_node_t));
        pub->account = publisher;
        list_add_tail(&pub->node, &account->followers_list);
        /* let the publiser list add the account */
        memset(sub, 0, sizeof(account_node_t));
        sub->account = account;
        list_add_tail(&sub->node, &publisher->fans_list);

        DATA_CENTER_TRACE("fans[%s] following uploader[%s] success", account->id, pub_id);
        error_flag = 0;
    } while (0);

    switch (error_flag) {
        case 1:
            __free(pub);
            break;
        case 2:
            __free(pub);
            __free(sub);
            break;
        default:
            break;
    }
    return publisher;
}

bool account_unsubscribe(account_t *account, const char *pub_id)
{
    bool retval = false;
    do {
        if (account == NULL || pub_id == NULL) {
            break;
        }

        account_t *pub = _search_account(account->center, pub_id);
        if (pub == NULL) {
            DATA_CENTER_TRACE("account[%s] was not followed [%s]", account->id, pub_id);
            break;
        }
        _datacenter_remove(&pub->fans_list, account);
        _datacenter_remove(&account->followers_list, pub);
        retval = true;
        DATA_CENTER_TRACE("account[%s] unfollowed %s", account->id, pub_id);
    } while (0);
    return retval;
}

bool account_commit(account_t *account, const void *data, unsigned int size)
{
    bool retval = false;
    do {
        if (!size || size != account->priv.buffer_size || account == NULL) {
            break;
        }
        void *wbuf;
        pingpong_buffer_get_write_buf(&account->priv.buffer_manager, &wbuf);
        memcpy(wbuf, data, size);
        pingpong_buffer_set_write_done(&account->priv.buffer_manager);
        DATA_CENTER_TRACE("account[%s] commit data(0x%p)[%d] >> data(0x%p)[%d] done", account->id, data, size, wbuf, size);
        retval = true;
    } while (0);
    return retval;
}

/**
 * @brief Publishes data to all fans (a broadcasting mechanism, requires prior invocation of account_commit).
 * @param {account_t} *account Pointer to the account structure.
 * @return {int} Returns error code
 */
int account_pubilsh(account_t *account)
{
    int retval = -1;
    do {
        if (account == NULL) {
            break;
        }
        if (account->priv.buffer_size == 0) {
            DATA_CENTER_TRACE("account[%s] has no cache", account->id);
            retval = ACCOUNT_RES_CODE_NO_CACHE;
            break;
        }
        void *rbuf;
        if (!pingpong_buffer_get_read_buf(&account->priv.buffer_manager, &rbuf)) {
            DATA_CENTER_TRACE("account[%s] has not commit", account->id);
            retval = ACCOUNT_RES_NO_COMMITED;
            break;
        }

        account_event_param_t param;
        param.event = ACCOUNT_EVENT_PUB_PUBLISH;
        param.tran = account;
        param.recv = NULL;
        param.data = rbuf;
        param.len = account->priv.buffer_size;

        account_node_t *pos, *n;
        list_for_each_entry_safe(pos, n, account_node_t, &account->fans_list, node)
        {
            account_t *fan = pos->account;
            event_callback_t cb = fan->priv.event_call_back;
            DATA_CENTER_TRACE("account[%s] publish >> data(0x%p)[%d] >> fans[%s]...", account->id, param.data, param.len, fan->id);
            if (cb != NULL) {
                param.recv = fan;
                int ret = cb(fan, &param);
                DATA_CENTER_TRACE("publish done: %d", ret);
                retval = ret;
            } else {
                DATA_CENTER_TRACE("fans[%s] has no callback", fan->id);
            }
        }
        pingpong_buffer_set_read_done(&account->priv.buffer_manager);
    } while (0);
    return retval;
}

int account_pull_from_account(account_t *account, account_t *pub, void *data, unsigned int size)
{
    int retval = ACCOUNT_RES_UNKNOW;
    do {
        if (account == NULL || pub == NULL) {
            retval = ACCOUNT_RES_NOT_FOUND;
            break;
        }
        DATA_CENTER_TRACE("account[%s] pull data(0x%p)[%d] from %s", account->id, data, size, pub->id);
        event_callback_t cb = pub->priv.event_call_back;
        if (cb != NULL) {
            account_event_param_t param;
            param.event = ACCOUNT_EVENT_SUB_PULL;
            param.tran = account;
            param.recv = pub;
            param.data = data;
            param.len = size;
            int ret = cb(pub, &param);
            DATA_CENTER_TRACE("pull done: %d", ret);
            retval = ret;
        } else {
            DATA_CENTER_TRACE("pub[%s] has not registed pull callback, read commit cache...", pub->id);
            if (pub->priv.buffer_size == size) {
                void *rbuf;
                if (pingpong_buffer_get_read_buf(&pub->priv.buffer_manager, &rbuf)) {
                    memcpy(data, rbuf, size);
                    pingpong_buffer_set_read_done(&pub->priv.buffer_manager);
                    DATA_CENTER_TRACE("read done");
                    retval = 0;
                } else {
                    DATA_CENTER_TRACE("pub[%s] has not commit", pub->id);
                }
            } else {
                DATA_CENTER_TRACE("data size pub[%s]:%d != sub[%s]:%d", pub->id, pub->priv.buffer_size, account->id, size);
            }
        }
    } while (0);
    return retval;
}

int account_pull_from_id(account_t *account, const char *pub_id, void *data, unsigned int size)
{
    account_t *pub = _search_account(account->center, pub_id);
    if (pub == NULL) {
        DATA_CENTER_TRACE("account[%s] was not followed [%s]", account->id, pub_id);
        return ACCOUNT_RES_NOT_FOUND;
    }
    return account_pull_from_account(account, pub, data, size);
}

int account_notify_from_account(account_t *account, account_t *pub, void *data, unsigned int size)
{
    int retval = ACCOUNT_RES_UNKNOW;
    do {
        if (account == NULL || pub == NULL) {
            retval = ACCOUNT_RES_NOT_FOUND;
            break;
        }
        DATA_CENTER_TRACE("account[%s] notify data(0x%p)[%d] to [%s]", account->id, data, size, pub->id);
        event_callback_t cb = pub->priv.event_call_back;
        if (cb != NULL) {
            account_event_param_t param;
            param.event = ACCOUNT_EVENT_NOTIFY;
            param.tran = account;
            param.recv = pub;
            param.data = data;
            param.len = size;
            int ret = cb(pub, &param);
            DATA_CENTER_TRACE("notify done: %d", ret);
            retval = ret;
        } else {
            DATA_CENTER_TRACE("pub[%s] has not registed notify callback", pub->id);
            retval = ACCOUNT_RES_CODE_NO_CALLBACK;
        }
    } while (0);
    return retval;
}

int account_notify_from_id(account_t *account, const char *pub_id, void *data, unsigned int size)
{
    account_t *pub = _search_account(account->center, pub_id);
    if (pub == NULL) {
        DATA_CENTER_TRACE("account[%s] was not followed [%s]", account->id, pub_id);
        return ACCOUNT_RES_NOT_FOUND;
    }
    return account_notify_from_account(account, pub, data, size);
}

void account_set_event_callback(account_t *account, event_callback_t cb)
{
    if (account != NULL) {
        account->priv.event_call_back = cb;
    }
}

static void account_timer_callback_handler(fp_timer_t *timer)
{
    account_t *account = (account_t *)(timer->user_data);
    event_callback_t cb = account->priv.event_call_back;
    if (cb != NULL) {
        account_event_param_t param;
        param.event = ACCOUNT_EVENT_TIMER;
        param.tran = account;
        param.recv = account;
        param.data = NULL;
        param.len = 0;
        cb(account, &param);
    }
}

void account_set_timer_period(account_t *account, uint32_t period)
{
    if (account != NULL && account->priv.timer != NULL) {
        fp_timer_del(account->priv.timer);
        account->priv.timer = NULL;
    }

    if (period == 0) {
        return;
    }
    account->priv.timer = fp_timer_create(account_timer_callback_handler, period, account);
}

void account_set_timer_enable(account_t *account, bool en)
{
    fp_timer_t *timer = account->priv.timer;

    if (timer == NULL) {
        return;
    }
    en ? fp_timer_resume(timer) : fp_timer_pasue(timer);
}
/*---------- end of file ----------*/
