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

#include "account.h"
#include "stdlib.h"
/*---------- macro ----------*/



#define ACCOUNT_MAX_ID_LENGTH 16
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

account_t *account_init(const char *id, data_center_t *center, unsigned int buffer_size, void *user_data)
{
    account_t *new = malloc(sizeof(account_t));
    bool error = false;
    do {
        if (new == NULL) {
            DATA_CENTER_TRACE("malloc new account failed.\n");
            break;
        }

        memset(&new->priv, 0, sizeof(new->priv));
        new->id = id;
        new->center = center;
        new->user_data = user_data;

        if (buffer_size != 0) {
            unsigned char *buffer = malloc(buffer_size);
            if (buffer == NULL) {
                DATA_CENTER_TRACE("account[%s] buffer malloc failed\n",id);
                error = true;
                break;
            }
            memset(buffer, 0 ,buffer_size * sizeof(unsigned char) * 2);
            unsigned char *buf0 = buffer;
            unsigned char *buf1 = buffer + buffer_size;
            pingpong_buffer_init(&new->priv.buffer_manager, buf0, buf1, buffer_size);
            new->priv.buffer_size = buffer_size;
            DATA_CENTER_TRACE("account[%s] cached %d x2 bytes", id, buffer_size);
        }

        center->ops.add_account(new);
        DATA_CENTER_TRACE("account[%s] created", id);
    } while (0);

    if (error == true) {
        free(new);
        new = NULL;
    }
    return new;
}

void account_deinit(account_t *account)
{
    DATA_CENTER_TRACE("account[%s] deleting...\n", account->id);
    /* release cache */
    if (account->priv.buffer_size) {
        free(account->priv.buffer_manager.buffer[0]);
    }
    /* delete timer */
    if (account->priv.timer) {
        fp_timer_del(account->priv.timer);
        DATA_CENTER_TRACE("account[%s] task deleted\n", account->id);
    }
    /* let subscribers unfollow */
    /* ask the publisher to delete this subscriber */
    /* let the data center delete the account */
    account->center->ops.remove_account(account);
    DATA_CENTER_TRACE("account[%s] deleted\n", account->id);
}

/*---------- end of file ----------*/


