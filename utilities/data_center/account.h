/**
 * @file common/data_center/inc/account.h
 *
 * Copyright (C) 2022
 *
 * account.h is free software: you can redistribute it and/or modify
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
#ifndef __ACCOUNT_H
#define __ACCOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "pingpong.h"
#include "soft_timer.h"
#include "clists.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
enum account_event {
    ACCOUNT_EVENT_NONE,
    ACCOUNT_EVENT_PUB_PUBLISH, /*<< Publisher posted information */
    ACCOUNT_EVENT_SUB_PULL,    /*<< Subscriber data pull request */
    ACCOUNT_EVENT_NOTIFY,      /*<< Subscriber send notifications to publishers */
    ACCOUNT_EVENT_TIMER,       /*<< Timed event */
    __ACCOUNT_EVENT_LAST
};

enum account_error {
    ACCOUNT_ERR_NONE = 0,
    ACCOUNT_ERR_UNKNOW,
    ACCOUNT_ERR_SIZE_MISMATCH,
    ACCOUNT_ERR_UNSUPPORTED_REQUEST,
    ACCOUNT_ERR_NO_CALLBACK,
    ACCOUNT_ERR_NO_CACHE,
    ACCOUNT_ERR_NO_COMMITED,
    ACCOUNT_ERR_NOT_FOUND,
    ACCOUNT_ERR_PARAM_ERROR
};

typedef struct account *account_t;
struct account_event_param {
    enum account_event event; /*<< event type */
    account_t tran;           /*<< pointer to sender */
    account_t recv;           /*<< pointer to receive */
    void *data;               /*<< pointer to data */
    uint32_t size;            /*<< the length of the data */
};

/* event callback
 */
typedef int32_t (*account_event_cb_t)(account_t account, struct account_event_param *param);

typedef struct data_center *data_center_t;
struct account {
    const char *id;       /*<< Unique account id */
    data_center_t center; /*<< pointer to the data center */
    void *user_data;
    struct list_head publishers;
    struct list_head subscribers;
    struct {
        account_event_cb_t event_cb;
        fp_timer_t *timer;
        struct pingpong_buffer buffer_manager;
        uint32_t buffer_size;
    } priv;
    /* operate functions */
    struct {
        /**
         * @brief Subscribe to Publisher.
         * @param account: Pointer to the subscriber's account.
         * @param pub_id: Pointer to the name of the publisher.
         * @retval Return the pointer to the publisher if subscribe success,
         *         otherwise, return NULL.
         */
        account_t (*subscribe)(account_t account, const char *pub_id);
        /**
         * @brief Unsubscribe from publisher.
         * @param account: Pointer to the subscriber's account.
         * @param pub_id: Pointer to the name of the publisher.
         * @retval Return true if unsubscribe success, otherwise, return false.
         */
        bool (*unsubscribe)(account_t account, const char *pub_id);
        /**
         * @brief Submit data to the cache.
         * @param account: Pointer to the publisher's account.
         * @param data: Pointer to the data.
         * @param size: The size of the data.
         * @retval Submit data success, return true.
         *         Publisher has no cache or size mismatch, return false.
         */
        bool (*commit)(account_t account, const void *data, uint32_t size);
        /**
         * @brief Publish data to subscribers.
         * @param account: Pointer to the publisher's account.
         * @retval @type error_code_en_t
         */
        int32_t (*publish)(account_t account);
        /**
         * @brief Pull data from the publisher.
         * @param account: Pointer to the subscriber's account.
         * @param pub_id: Pointer to the name of the publisher.
         * @param data: Pointer to the data.
         * @param size: The length of the data.
         * @retval @type error_code_en_t
         */
        int32_t (*pull)(account_t account, const char *pub_id, void *data, uint32_t size);
        /**
         * @brief Send a notification to the publisher.
         * @param account: Pointer to the subscriber's account.
         * @param pub_id: Pointer to the name of the publisher.
         * @param data: Pointer to the data.
         * @param size: The length of the data.
         * @retval @type error_code_en_t
         */
        int32_t (*notify)(account_t account, const char *pub_id, const void *data, uint32_t size);
        /**
         * @brief Set event callback.
         * @param account: Pointer to the account.
         * @param cb: Pointer to the callback function.
         * @retval None
         */
        void (*set_event_cb)(account_t account, account_event_cb_t cb);
        /**
         * @brief Set timer period
         * @param account: Pointer to the account.
         * @param period: The period of the timer.
         * @retval None
         */
        void (*set_timer_period)(account_t account, uint32_t period);
        /**
         * @brief Set timer enable.
         * @param account: Pointer to the account.
         * @param en: Whether to enable.
         * @retval None
         */
        void (*set_timer_enable)(account_t account, bool en);
        /**
         * @brief Get the number of the publishers
         * @param account: Pointer of the account.
         * @retval number of the publishers.
         */
        uint32_t (*get_publisher_size)(account_t account);
        /**
         * @brief Get the number of the subscribers.
         * @param account: Pointer of the account.
         * @retval number of the subscribers.
         */
        uint32_t (*get_subscriber_size)(account_t account);
    } ops;
};

struct account_node {
    account_t account;
    struct list_head node;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief Initialize account structures.
 * @param account The handle of the account that will be initialized.
 * @param id Account's name.
 * @param center The handle of the data center that account will be mounted to.
 * @param buf_size Account's buffer size.
 * @param user_data The handle of the accout's priv data.
 *
 * @retval If initialize successfully then true is returned, otherwise false is returned.
 */
extern bool account_create(account_t account, const char *id, data_center_t center, uint32_t buf_size, void *user_data);

/**
 * @brief Deinitialize account structures.
 * @param account The handle of the account that will be deinititialized.
 *
 * @retval None
 */
extern void account_destroy(account_t account);

#ifdef __cplusplus
}
#endif
#endif /* __ACCOUNT_H */
