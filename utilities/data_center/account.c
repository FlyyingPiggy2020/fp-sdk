/**
 * @file common/data_center/account.c
 *
 * Copyright (C) 2022
 *
 * account.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#define LOG_TAG "Account"
#include "account.h"
#include "data_center.h"
#include "options.h"
#include "soft_timer.h"
#include <string.h>

/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static bool inline __match_by_name(const char *s1, const char *s2)
{
    return (strcmp(s1, s2) == 0);
}

static account_t _subscribe(account_t account, const char *pub_id)
{
    account_t pub = NULL;
    struct account_node *publisher = NULL, *subscriber = NULL;

    do {
        if (account == NULL || pub_id == NULL) {
            break;
        }
        if (__match_by_name(account->id, pub_id) == true) {
            log_e("%s try to subscribe to itself\n", pub_id);
            break;
        }
        pub = account->center->ops.find(&account->publishers, pub_id);
        if (pub != NULL) {
            log_e("multi subscribe pub(%s)\n", pub_id);
            pub = NULL;
            break;
        }
        pub = account->center->ops.search_account(account->center, pub_id);
        if (pub == NULL) {
            log_e("pub(%s) was not found\n", pub_id);
            break;
        }
        publisher = __malloc(sizeof(struct account_node));
        if (publisher == NULL) {
            log_e("alloc memory for pub(%s) publish account(%s) failed\n", pub_id, account->id);
            pub = NULL;
            break;
        }
        subscriber = __malloc(sizeof(struct account_node));
        if (subscriber == NULL) {
            log_e("alloc memory for sub(%s) subscribe account(%s) failed\n", account->id, pub_id);
            __free(publisher);
            pub = NULL;
            break;
        }
        /* Add the publisher to the subscription list */
        memset(publisher, 0, sizeof(*publisher));
        publisher->account = pub;
        list_add_tail(&publisher->node, &account->publishers);
        /* let the publisher add this subscriber */
        memset(subscriber, 0, sizeof(*subscriber));
        subscriber->account = account;
        list_add_tail(&subscriber->node, &pub->subscribers);
        log_i("sub(%s) subscribed pub(%s)\n", account->id, pub_id);
    } while (0);

    return pub;
}

static bool _unsubscribe(account_t account, const char *pub_id)
{
    account_t pub = NULL;
    bool retval = false;

    do {
        if (account == NULL || pub_id == NULL) {
            break;
        }
        pub = account->center->ops.find(&account->publishers, pub_id);
        if (pub == NULL) {
            break;
        }
        /* Remove the publisher from the subscription list */
        account->center->ops.remove(&account->publishers, pub);
        /* Let the publisher remove this subscriber */
        account->center->ops.remove(&pub->subscribers, account);
        retval = true;
    } while (0);

    return retval;
}

static bool _commit(account_t account, const void *data, uint32_t size)
{
    bool retval = false;
    void *wbuf = NULL;

    do {
        if (size == 0 || size != account->priv.buffer_size) {
            log_e("pub(%s) has not cache or size mismatch\n", account->id);
            break;
        }
        /* subcommit data to cache */
        pingpong_buffer_get_write_buf(&account->priv.buffer_manager, &wbuf);
        memcpy(wbuf, data, size);
        pingpong_buffer_set_write_done(&account->priv.buffer_manager);
        log_i("pub(%s) commit data(0x%p)[%d] >> data(0x%p)[%d] done\n", account->id, data, size, wbuf, size);
        retval = true;
    } while (0);

    return retval;
}

static int32_t _publish(account_t account)
{
    int32_t retval = -ACCOUNT_ERR_UNKNOW;
    void *rbuf = NULL;
    struct account_event_param param = { 0 };
    struct account_node *subscriber = NULL;

    do {
        if (account == NULL) {
            retval = -ACCOUNT_ERR_PARAM_ERROR;
            break;
        }
        if (account->priv.buffer_size == 0) {
            log_e("pub(%s) has no cache\n", account->id);
            retval = -ACCOUNT_ERR_NO_CACHE;
            break;
        }
        if (pingpong_buffer_get_read_buf(&account->priv.buffer_manager, &rbuf) == false) {
            log_e("pub(%s) data was not commit\n", account->id);
            retval = -ACCOUNT_ERR_NO_COMMITED;
            break;
        }
        param.event = ACCOUNT_EVENT_PUB_PUBLISH;
        param.tran = account;
        param.recv = NULL;
        param.data = rbuf;
        param.size = account->priv.buffer_size;
        /* push message to all subscribers */
        list_for_each_entry(subscriber, struct account_node, &account->subscribers, node)
        {
            account_t sub = subscriber->account;
            account_event_cb_t cb = sub->priv.event_cb;
            log_i("pub(%s) push >> data(0x%p)[%d] >> sub(%s)\n", account->id, param.data, param.size, sub->id);
            if (cb) {
                param.recv = sub;
                retval = cb(sub, &param);
                log_i("push done: %d\n", retval);
            } else {
                log_i("sub(%s) not register callback\n", account->id);
            }
        }
        pingpong_buffer_set_read_done(&account->priv.buffer_manager);
    } while (0);

    return retval;
}

static int32_t _pull_from_publisher(account_t sub, account_t pub, void *data, uint32_t size)
{
    int32_t retval = -ACCOUNT_ERR_NOT_FOUND;
    void *rbuf = NULL;
    account_event_cb_t cb = NULL;

    do {
        if (pub == NULL) {
            break;
        }
        log_i("sub(%s) pull << data(0x%p)[%d] << pub(%s)\n", sub->id, data, size, pub->id);
        cb = pub->priv.event_cb;
        if (cb) {
            struct account_event_param param = { 0 };
            param.event = ACCOUNT_EVENT_SUB_PULL;
            param.tran = sub;
            param.recv = pub;
            param.data = data;
            param.size = size;
            retval = cb(pub, &param);
            log_i("pull done: %d\n", retval);
            break;
        }
        log_i("pub(%s) not register pull callback, read commit cache\n", pub->id);
        if (pub->priv.buffer_size != size) {
            log_e("data size pub(%s): %d != sub(%s): %d\n", pub->id, pub->priv.buffer_size, sub->id, size);
            break;
        }
        if (pingpong_buffer_get_read_buf(&pub->priv.buffer_manager, &rbuf) == true) {
            memcpy(data, rbuf, size);
            pingpong_buffer_set_read_done(&pub->priv.buffer_manager);
            log_i("read done\n");
            retval = ACCOUNT_ERR_NONE;
        } else {
            log_w("pub(%s) data was not commit\n", pub->id);
        }
    } while (0);

    return retval;
}

static int32_t _pull(account_t account, const char *pub_id, void *data, uint32_t size)
{
    int32_t retval = -ACCOUNT_ERR_NOT_FOUND;
    account_t pub = NULL;

    do {
        if (account == NULL || pub_id == NULL || data == NULL) {
            break;
        }
        pub = account->center->ops.find(&account->publishers, pub_id);
        if (pub == NULL) {
            log_e("sub(%s) was not subscribe pub(%s)\n", account->id, pub_id);
            break;
        }
        retval = _pull_from_publisher(account, pub, data, size);
    } while (0);

    return retval;
}

static int32_t _notify_publisher(account_t sub, account_t pub, const void *data, uint32_t size)
{
    int32_t retval = -ACCOUNT_ERR_NOT_FOUND;
    account_event_cb_t cb = NULL;
    struct account_event_param param = { 0 };

    do {
        if (pub == NULL) {
            break;
        }
        log_i("sub(%s) notify >> data(0x%p)[%d] >> pub(%s)\n", sub->id, data, size, pub->id);
        cb = pub->priv.event_cb;
        if (cb == NULL) {
            log_w("pub(%s) not register callback\n", pub->id);
            retval = -ACCOUNT_ERR_NO_CALLBACK;
            break;
        }
        param.event = ACCOUNT_EVENT_NOTIFY;
        param.tran = sub;
        param.recv = pub;
        param.data = (void *)data;
        param.size = size;
        retval = cb(pub, &param);
        log_i("notify done: %d\n", retval);
    } while (0);

    return retval;
}

static int32_t _notify(account_t account, const char *pub_id, const void *data, uint32_t size)
{
    int32_t retval = -ACCOUNT_ERR_NOT_FOUND;
    account_t pub = NULL;

    do {
        if (account == NULL || pub_id == NULL || data == NULL) {
            break;
        }
        pub = account->center->ops.find(&account->publishers, pub_id);
        if (pub == NULL) {
            log_e("sub(%s) was not subscribe pub(%s)\n", account->id, pub_id);
            break;
        }
        retval = _notify_publisher(account, pub, data, size);
    } while (0);

    return retval;
}

static void _set_event_callback(account_t account, account_event_cb_t cb)
{
    if (account) {
        account->priv.event_cb = cb;
    }
}

static uint32_t _get_publisher_count(account_t account)
{
    uint32_t count = 0;
    struct list_head *p = NULL;

    if (account) {
        list_for_each(p, &account->publishers)
        {
            count++;
        }
    }

    return count;
}

static void _timer_callback_handler(struct _fp_timer_t *timer)
{
    account_t account = (account_t)timer->user_data;
    account_event_cb_t cb = account->priv.event_cb;
    struct account_event_param evt = { 0 };

    if (cb) {
        evt.event = ACCOUNT_EVENT_TIMER;
        evt.tran = account;
        evt.recv = account;
        evt.data = NULL;
        evt.size = 0;
        cb(account, &evt);
    }
}

static void _set_timer_period(account_t account, uint32_t period)
{
    if (account) {
        if (account->priv.timer) {
            fp_timer_del(account->priv.timer);
            account->priv.timer = NULL;
        }
        if (period) {
            account->priv.timer = fp_timer_create(_timer_callback_handler, period, &account);
        }
    }
}

static void _set_timer_enable(account_t account, bool en)
{
    fp_timer_t *timer = account->priv.timer;

    if (timer) {
        if (en) {
            fp_timer_reset(timer);
        } else {
            fp_timer_pasue(timer);
        }
    }
}

static uint32_t _get_subscriber_count(account_t account)
{
    uint32_t count = 0;
    struct list_head *p = NULL;

    if (account) {
        list_for_each(p, &account->subscribers)
        {
            count++;
        }
    }

    return count;
}

bool account_create(account_t account, const char *id, data_center_t center, uint32_t buf_size, void *user_data)
{
    bool retval = false;

    do {
        if (account == NULL || center == NULL) {
            break;
        }
        memset(&account->priv, 0, sizeof(account->priv));
        account->id = id;
        account->center = center;
        account->user_data = user_data;
        INIT_LIST_HEAD(&account->publishers);
        INIT_LIST_HEAD(&account->subscribers);
        /* bind ops */
        account->ops.subscribe = _subscribe;
        account->ops.unsubscribe = _unsubscribe;
        account->ops.commit = _commit;
        account->ops.publish = _publish;
        account->ops.pull = _pull;
        account->ops.notify = _notify;
        account->ops.set_event_cb = _set_event_callback;
        account->ops.set_timer_period = _set_timer_period;
        account->ops.set_timer_enable = _set_timer_enable;
        account->ops.get_publisher_size = _get_publisher_count;
        account->ops.get_subscriber_size = _get_subscriber_count;
        if (buf_size != 0) {
            uint8_t *buf0 = __malloc(buf_size);
            if (buf0 == NULL) {
                log_e("%s buf0 alloc failed\n", id);
                break;
            }
            uint8_t *buf1 = __malloc(buf_size);
            if (buf1 == NULL) {
                __free(buf0);
                log_e("%s buf1 alloc failed\n", id);
                break;
            }
            memset(buf0, 0, buf_size);
            memset(buf1, 0, buf_size);
            pingpong_buffer_init(&account->priv.buffer_manager, buf0, buf1);
            log_i("%s cached %d x2 bytes\n", id, buf_size);
            account->priv.buffer_size = buf_size;
        }
        center->ops.add_account(center, account);
        retval = true;
        log_i("%s created\n", id);
    } while (0);

    return retval;
}

void account_destroy(account_t account)
{
    struct account_node *p = NULL, *n = NULL;
    account_t sub = NULL;
    account_t pub = NULL;

    if (account) {
        log_i("account(%s) destroy...\n", account->id);
        /* release cache */
        if (account->priv.buffer_size) {
            __free(account->priv.buffer_manager.buffer[0]);
            __free(account->priv.buffer_manager.buffer[1]);
            memset(&account->priv.buffer_manager, 0, sizeof(account->priv.buffer_manager));
            account->priv.buffer_size = 0;
        }
        /* destroy timer */
        if (account->priv.timer) {
            fp_timer_del(account->priv.timer);
            account->priv.timer = NULL;
            log_i("account(%s) timer delete\n", account->id);
        }
        /* let subscribers unfollow */
        list_for_each_entry_safe(p, n, struct account_node, &account->subscribers, node)
        {
            sub = p->account;
            sub->ops.unsubscribe(sub, account->id);
            log_i("sub(%s) unsubscribe pub(%s)\n", sub->id, account->id);
        }
        /* ask the publisher to delete this subscriber */
        list_for_each_entry_safe(p, n, struct account_node, &account->publishers, node)
        {
            pub = p->account;
            account->center->ops.remove(&pub->subscribers, account);
            list_del(&p->node);
            __free(p);
            log_i("pub(%s) remove sub(%s)\n", pub->id, account->id);
        }
        account->center->ops.remove_account(account->center, account);
        log_i("account(%s) destroy\n", account->id);
    }
}
