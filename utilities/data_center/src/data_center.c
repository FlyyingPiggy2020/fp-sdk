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
 * @FilePath     : data_center.c
 * @Author       : lxf
 * @Date         : 2024-07-19 14:29:17
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-19 17:05:54
 * @Brief        :
 */

/*---------- includes ----------*/
#include "stdlib.h"
#include "string.h"
#include "data_center.h"
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

data_center_t *data_center_init(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    data_center_t *new = __malloc(sizeof(data_center_t));
    memset(new, 0, sizeof(data_center_t));
    if (new == NULL) {
        DATA_CENTER_TRACE("malloc new data center failed\n");
        return NULL;
    }
    new->name = name;
    INIT_LIST_HEAD(&new->account_pool);
    new->account_main = account_init(name, new, 0, NULL);
    return new;
}

void data_center_deinit(data_center_t *center)
{
    if (center == NULL) {
        return;
    }
    DATA_CENTER_TRACE("data center[%s] closing.\n", center->name);
    account_node_t *p, *n;
    list_for_each_entry_safe(p, n, account_node_t, &center->account_pool, node)
    {
        account_t *account = p->account;
        DATA_CENTER_TRACE("delete:%s\n", account->id);
        account_deinit(account);
    }
    memset(center, 0, sizeof(data_center_t));
    INIT_LIST_HEAD(&center->account_pool);
    __free(center);
}

account_t *_datacenter_find(struct list_head *pool, const char *id)
{
    account_t *account = NULL;
    account_node_t *p = NULL;
    list_for_each_entry(p, account_node_t, pool, node)
    {
        if (p->account == NULL) {
            DATA_CENTER_TRACE("account is null\n");
            continue;
        } else if (p->account->id == NULL){
            DATA_CENTER_TRACE("account:%p id is null\n",p->account);
            continue;
        }else {
            DATA_CENTER_TRACE("account id:%s,%p\n",p->account->id, p->account);
        }

        if (__match_by_name(id, p->account->id) == true) {
            account = p->account;
            break;
        }
    }
    return account;
}
account_t *_search_account(data_center_t *center, const char *id)
{
    return _datacenter_find(&center->account_pool, id);
}

bool datacenter_add_account(data_center_t *center, account_t *account)
{
    bool retval = false;
    account_node_t *p = NULL;
    do {
        if (center == NULL || account == NULL) {
            break;
        }

        if (account == center->account_main) {
            DATA_CENTER_TRACE("account main can't added itself\n");
            break;
        }
        if (_search_account(center, account->id) != NULL) {
            DATA_CENTER_TRACE("account[%s] already exists.\n", account->id);
            break;
        }
        
        p = __malloc(sizeof(account_node_t));
        if (p == NULL) {
            DATA_CENTER_TRACE("malloc account_node_t failed.\n");
            break;
        }
        memset(p, 0, sizeof(account_node_t));
        p->account = account;
        DATA_CENTER_TRACE("add[%s:%p] to account pool.node:%p\n", p->account->id, p->account, p);
        list_add_tail(&p->node, &center->account_pool);
        // _search_account(account->center, account->id);
        
        account_subscribe(center->account_main, account->id);
        DATA_CENTER_TRACE("add account[%s] to data center[%s].\n", account->id, center->name);
        retval = true;
    } while (0);
    return retval;
}

bool _datacenter_remove(struct list_head *pool, account_t *account)
{
    account_node_t *p, *n;
    bool retval = false;

    do {
        if (account == NULL || pool == NULL) {
            break;
        }
        list_for_each_entry_safe(p, n, account_node_t, pool, node)
        {
            if (__match_by_name(p->account->id, account->id) == true) {
                list_del(&p->node);
                __free(p);
                retval = true;
                break;
            }
        }
        if (retval == false) {
            DATA_CENTER_TRACE("account[%s] was not found.\n", account->id);
        }
    } while (0);

    return retval;
}

bool datacenter_remove_account(data_center_t *center, account_t *account)
{
    return _datacenter_remove(&center->account_pool, account);
}

uint32_t datacenter_get_account_count(data_center_t *center)
{
    uint32_t count = 0;
    account_node_t *p, *n;
    if (center != NULL) {
        list_for_each_entry_safe(p, n, account_node_t, &center->account_pool, node)
        {
            count++;
        }
    }
    return count;
}
/*---------- end of file ----------*/
