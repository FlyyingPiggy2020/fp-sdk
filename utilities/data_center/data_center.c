/**
 * @file common/data_center/data_center.c
 *
 * Copyright (C) 2022
 *
 * data_center.c is free software: you can redistribute it and/or modify
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
#define LOG_TAG "DataCenter"

#include "data_center.h"
#include "options.h"
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

static account_t _find(struct list_head *pool, const char *id)
{
    account_t account = NULL;
    struct account_node *p = NULL;

    list_for_each_entry(p, struct account_node, pool, node)
    {
        if (__match_by_name(id, p->account->id) == true) {
            account = p->account;
            break;
        }
    }

    return account;
}

static account_t _search_account(data_center_t center, const char *id)
{
    return _find(&center->account_pool, id);
}

static bool _add_account(data_center_t center, account_t account)
{
    bool retval = false;
    struct account_node *p = NULL;

    do {
        if (center == NULL || account == NULL) {
            break;
        }
        if (account == &center->account_main) {
            log_w("Account Main(%s) can not add itself\n", account->id);
            break;
        }
        if (_search_account(center, account->id) != NULL) {
            log_e("multi add Account(%s)\n", account->id);
            break;
        }
        p = __malloc(sizeof(struct account_node));
        if (p == NULL) {
            log_e("alloc memory for Account(%s) to add account pool failed\n", account->id);
            break;
        }
        log_i("alloc 0x%p for new Account(%s) to add account pool\n", p, account->id);
        /* push account to account pool */
        memset(p, 0, sizeof(*p));
        p->account = account;
        list_add_tail(&p->node, &center->account_pool);
        /* account main subscribe account */
        center->account_main.ops.subscribe(&center->account_main, account->id);
        retval = true;
    } while (0);

    return retval;
}

static bool _remove(struct list_head *pool, account_t account)
{
    struct account_node *p = NULL, *n = NULL;
    bool retval = false;

    do {
        if (account == NULL || pool == NULL) {
            break;
        }
        list_for_each_entry_safe(p, n, struct account_node, pool, node)
        {
            if (__match_by_name(p->account->id, account->id) == true) {
                log_i("remove account(%s) from account pool at 0x%p ok\n", account->id, p);
                list_del(&p->node);
                __free(p);
                retval = true;
                break;
            }
        }
        if (retval == false) {
            log_e("account(%s) was not found in account pool\n", account->id);
        }
    } while (0);

    return retval;
}

static bool _remove_account(data_center_t center, account_t account)
{
    bool retval = false;

    if (center != NULL) {
        retval = _remove(&center->account_pool, account);
    }

    return retval;
}

static uint32_t _get_account_count(data_center_t center)
{
    uint32_t count = 0;
    struct list_head *p = NULL;

    if (center != NULL) {
        list_for_each(p, &center->account_pool)
        {
            count++;
        }
    }

    return count;
}

void data_center_init(data_center_t center, const char *name)
{
    assert(center);
    assert(name);
    center->name = name;
    center->ops.add_account = _add_account;
    center->ops.remove_account = _remove_account;
    center->ops.remove = _remove;
    center->ops.search_account = _search_account;
    center->ops.find = _find;
    center->ops.get_account_count = _get_account_count;
    INIT_LIST_HEAD(&center->account_pool);
    account_create(&center->account_main, name, center, 0, NULL);
}

void data_center_deinit(data_center_t center)
{
    struct account_node *p = NULL, *n = NULL;

    assert(center);
    /* delete all accounts that have been mounted to the main account */
    list_for_each_entry_safe(p, n, struct account_node, &center->account_pool, node)
    {
        account_destroy(p->account);
        list_del(&p->node);
        __free(p);
    }
    /* delete main account */
    account_destroy(&center->account_main);
    memset((void *)center, 0, sizeof(struct data_center));
    INIT_LIST_HEAD(&center->account_pool);
}
