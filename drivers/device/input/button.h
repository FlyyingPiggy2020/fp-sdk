/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : components/fp-sdk/drivers/device/input/button.h
 * @Author       : Codex
 * @Date         : 2026-03-20 15:10:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 15:10:00
 * @Brief        : 通用按键设备驱动接口
 * @usage        :
 *               @code
 *               static bool key_k1_is_active(void *ctx)
 *               {
 *                   (void)ctx;
 *                   return HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_8) == GPIO_PIN_RESET;
 *               }
 *
 *               static bool key_k1k2_is_active(void *ctx)
 *               {
 *                   (void)ctx;
 *                   return key_k1_is_active(NULL) && key_k2_is_active(NULL);
 *               }
 *
 *               static const struct button_key_cfg s_button_keys[] = {
 *                   { .is_active = key_k1_is_active, .long_time = 100U, .repeat_speed = 6U },
 *                   { .is_active = key_k1k2_is_active, .long_time = 0U, .repeat_speed = 0U },
 *               };
 *
 *               static struct button_key_state s_button_states[ARRAY_SIZE(s_button_keys)];
 *               static uint32_t s_button_fifo[16];
 *
 *               static button_describe_t s_button_desc = {
 *                   .number_of_keys = ARRAY_SIZE(s_button_keys),
 *                   .filter_time = 5U,
 *                   .key_cfgs = s_button_keys,
 *                   .key_states = s_button_states,
 *                   .event_fifo = s_button_fifo,
 *                   .event_fifo_size = ARRAY_SIZE(s_button_fifo),
 *               };
 *
 *               DEVICE_DEFINED(user_key, button, &s_button_desc);
 *               @endcode
 * @note         扫描周期由外部决定, filter_time/long_time/repeat_speed 的单位均为扫描周期
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/
#define IOCTL_BUTTON_SCAN              (IOCTL_USER_START + 0x00)
#define IOCTL_BUTTON_GET_EVENT         (IOCTL_USER_START + 0x01)
#define IOCTL_BUTTON_CLEAR_EVENT       (IOCTL_USER_START + 0x02)
#define IOCTL_BUTTON_GET_STATE         (IOCTL_USER_START + 0x03)
#define IOCTL_BUTTON_SET_PARAM         (IOCTL_USER_START + 0x04)
#define IOCTL_BUTTON_SET_EVENT_CB      (IOCTL_USER_START + 0x05)

#define BUTTON_EVENT_CODE(key_id, evt) ((uint32_t)((key_id) * 4U + (uint32_t)(evt)))
#define BUTTON_EVENT_NONE_CODE         (0U)
/*---------- type define ----------*/
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_DOWN = 1,
    BUTTON_EVENT_SHORT_UP = 2,
    BUTTON_EVENT_LONG = 3,
    BUTTON_EVENT_LONG_UP = 4,
} button_event_t;

struct button_key_cfg {
    bool (*is_active)(void *ctx);
    void *ctx;
    uint16_t long_time;
    uint8_t repeat_speed;
};

struct button_key_state {
    uint16_t filter_count;
    uint16_t long_count;
    uint16_t long_time;
    uint8_t state;
    uint8_t repeat_speed;
    uint8_t repeat_count;
};

typedef struct {
    uint32_t number_of_keys;
    uint16_t filter_time;
    const struct button_key_cfg *key_cfgs;
    struct button_key_state *key_states;
    uint32_t *event_fifo;
    uint16_t event_fifo_size;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        void (*lock)(void *ctx);
        void (*unlock)(void *ctx);
    } ops;
    struct {
        int32_t (*event)(uint32_t event_code, void *ctx);
        void *ctx;
    } cb;
    struct {
        uint16_t fifo_read;
        uint16_t fifo_write;
    } priv;
} button_describe_t;

union button_ioctl_param {
    struct {
        uint32_t event;
    } get_event;
    struct {
        uint32_t key_id;
        bool state;
    } get_state;
    struct {
        uint32_t key_id;
        uint16_t long_time;
        uint8_t repeat_speed;
    } set_param;
    struct {
        int32_t (*event)(uint32_t event_code, void *ctx);
        void *ctx;
    } set_event_cb;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __BUTTON_H__ */
