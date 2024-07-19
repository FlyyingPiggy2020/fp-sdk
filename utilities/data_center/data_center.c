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

#include "data_center.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

data_center_t *data_center_init(const char *name)
{
    data_center_t *new = malloc(sizeof(data_center_t));
    new->name = name;
    return new;
}

void data_center_deinit(data_center_t *center)
{
    DATA_CENTER_TRACE("data center[%s] closing.\n", center->name);
    struct list_head *p, *n;
    list_for_each_safe(p, n, &center->account_pool) {
        account_t *account = list_entry(p, account_t, account_pool_node);
        DATA_CENTER_TRACE("delete:%s",account->id);
        account_deinit(account);
        list_del(p);
        free(account);
    }
    //TODO: free account
    free(center);
}
/*---------- end of file ----------*/


