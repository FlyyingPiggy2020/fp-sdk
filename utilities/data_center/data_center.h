/**
 * @file common/data_center/inc/data_center.h
 *
 * Copyright (C) 2022
 *
 * data_center.h is free software: you can redistribute it and/or modify
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
#ifndef __DATA_CENTER_H
#define __DATA_CENTER_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "account.h"
#include "clists.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
struct data_center {
    const char *name;            /*<< The name of the data center will be used as the ID of the main account */
    struct account account_main; /*<< Main account, will automaticatlly follow all accounts */
    struct list_head account_pool;
    struct {
        /**
         * @brief Add @account to the data center's main account. And then @account will mount to the main
         * account's publisher list, the main account will mount to the @account's subscriber list.
         * @param center The handle of data center.
         * @param account The handle of the account that will be added.
         *
         * @retval If @account has alrady exists or if the account node that will be mounted to the main account's
         * publisher list can not be created because there is insufficient heap remaining to allocate then
         * false is returned, otherwise, true is returned.
         */
        bool (*add_account)(data_center_t center, account_t account);
        /**
         * @brief Delete @account from the data center's main account.
         * @param center The handle of data center.
         * @param account The handle of the account that will be deleted.
         *
         * @retval If @account not found in the main account then false is returned.
         * If @account is deleted successfully then true is returned.
         */
        bool (*remove_account)(data_center_t center, account_t account);
        /**
         * @brief Delete @aacount from @pool list.
         * @param pool The handle of pool list.
         * @param account The handle of the account that will be deleted.
         *
         * @retval If @account not found in the @pool list then false is returned.
         * If @account is deleted successfully then true is returned.
         */
        bool (*remove)(struct list_head *pool, account_t account);
        /**
         * @brief Find a account in the @center by account name.
         * @param center The handle of data center.
         * @param id Account's name.
         *
         * @retval If account is found then the handle of the account is returned.
         * If account is not found then NULL is returned.
         */
        account_t (*search_account)(data_center_t center, const char *id);
        /**
         * @brief Find a account in the @pool list by account name.
         * @param pool The handle of the account pool list.
         * @param id Account's name.
         *
         * @retval If account is found then the handle of the account is returned.
         * If account is not found then NULL is returned.
         */
        account_t (*find)(struct list_head *pool, const char *id);
        /**
         * @brief Queries the number of the accounts that have been mounted to the
         * data center's main account.
         * @param center The handle of data center.
         *
         * @retval The number of the accounts.
         */
        uint32_t (*get_account_count)(data_center_t center);
    } ops;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief Initialize data center structures.
 * @param center The handle of the data center.
 * @param nmae Data center's name.
 *
 * @retval None
 */
extern void data_center_init(data_center_t center, const char *name);

/**
 * @brief Deinitialize data center structures.
 * @param center The handle of the data center.
 *
 * @retval None
 */
extern void data_center_deinit(data_center_t center);

#ifdef __cplusplus
}
#endif
#endif /* __DATA_CENTER_H */
