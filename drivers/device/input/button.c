/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : components/fp-sdk/drivers/device/input/button.c
 * @Author       : Codex
 * @Date         : 2026-03-20 15:10:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 16:20:00
 * @Brief        : 通用按键设备驱动
 */

/*---------- includes ----------*/
#include "options.h"
#include "driver.h"
#include "button.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _button_open(driver_t **pdrv);
static void _button_close(driver_t **pdrv);
static int32_t _button_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _button_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

static int32_t _ioctl_scan(button_describe_t *pdesc, void *args);
static int32_t _ioctl_get_event(button_describe_t *pdesc, void *args);
static int32_t _ioctl_clear_event(button_describe_t *pdesc, void *args);
static int32_t _ioctl_get_state(button_describe_t *pdesc, void *args);
static int32_t _ioctl_set_param(button_describe_t *pdesc, void *args);
static int32_t _ioctl_set_event_cb(button_describe_t *pdesc, void *args);

static void _button_reset_runtime(button_describe_t *pdesc);
static int32_t _button_scan_all(button_describe_t *pdesc);
static void _button_detect_one(button_describe_t *pdesc, uint32_t key_id);
static void _button_emit_event(button_describe_t *pdesc, uint32_t event_code);
static void _button_fifo_push(button_describe_t *pdesc, uint32_t event_code);
static bool _button_fifo_pop(button_describe_t *pdesc, uint32_t *event_code);
static void _button_fifo_clear(button_describe_t *pdesc);
/*---------- variable ----------*/
DRIVER_DEFINED(button, _button_open, _button_close, NULL, NULL, _button_ioctl, _button_irq_handler);

static const struct protocol_callback ioctl_cbs[] = {
    { IOCTL_BUTTON_SCAN, _ioctl_scan },
    { IOCTL_BUTTON_GET_EVENT, _ioctl_get_event },
    { IOCTL_BUTTON_CLEAR_EVENT, _ioctl_clear_event },
    { IOCTL_BUTTON_GET_STATE, _ioctl_get_state },
    { IOCTL_BUTTON_SET_PARAM, _ioctl_set_param },
    { IOCTL_BUTTON_SET_EVENT_CB, _ioctl_set_event_cb },
};
/*---------- function ----------*/
static int32_t _button_open(driver_t **pdrv)
{
    button_describe_t *pdesc = NULL;
    int32_t err = E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (pdesc == NULL) {
            break;
        }
        if ((pdesc->key_cfgs == NULL) || (pdesc->key_states == NULL)) {
            break;
        }
        if ((pdesc->number_of_keys == 0U) || (pdesc->event_fifo == NULL) || (pdesc->event_fifo_size == 0U)) {
            break;
        }

        if (pdesc->ops.init && !pdesc->ops.init()) {
            err = E_ERROR;
            break;
        }

        /* 每次打开设备都重置运行态，避免残留计数和 FIFO 影响本次使用。 */
        _button_reset_runtime(pdesc);
        err = E_OK;
    } while (0);

    return err;
}

static void _button_close(driver_t **pdrv)
{
    button_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if ((pdesc != NULL) && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t _button_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    button_describe_t *pdesc = NULL;
    int32_t err = E_WRONG_ARGS;
    int32_t (*cb)(button_describe_t *, void *) = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (pdesc == NULL) {
            break;
        }
        cb = (int32_t (*)(button_describe_t *, void *))protocol_callback_find(
            cmd, (void *)ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (cb == NULL) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}

static int32_t _button_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    button_describe_t *pdesc = NULL;

    (void)irq_handler;
    (void)args;
    (void)length;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc == NULL) {
        return E_WRONG_ARGS;
    }

    return _button_scan_all(pdesc);
}

static int32_t _ioctl_scan(button_describe_t *pdesc, void *args)
{
    (void)args;

    return _button_scan_all(pdesc);
}

static int32_t _ioctl_get_event(button_describe_t *pdesc, void *args)
{
    union button_ioctl_param *param = (union button_ioctl_param *)args;

    if (param == NULL) {
        return E_WRONG_ARGS;
    }

    param->get_event.event = BUTTON_EVENT_NONE_CODE;

    return _button_fifo_pop(pdesc, &param->get_event.event) ? E_OK : E_ERROR;
}

static int32_t _ioctl_clear_event(button_describe_t *pdesc, void *args)
{
    (void)args;

    _button_fifo_clear(pdesc);

    return E_OK;
}

static int32_t _ioctl_get_state(button_describe_t *pdesc, void *args)
{
    union button_ioctl_param *param = (union button_ioctl_param *)args;

    if (param == NULL) {
        return E_WRONG_ARGS;
    }
    if (param->get_state.key_id >= pdesc->number_of_keys) {
        param->get_state.state = false;
        return E_WRONG_ARGS;
    }

    param->get_state.state = (pdesc->key_states[param->get_state.key_id].state != 0U);

    return E_OK;
}

static int32_t _ioctl_set_param(button_describe_t *pdesc, void *args)
{
    union button_ioctl_param *param = (union button_ioctl_param *)args;
    struct button_key_state *state = NULL;

    if (param == NULL) {
        return E_WRONG_ARGS;
    }
    if (param->set_param.key_id >= pdesc->number_of_keys) {
        return E_WRONG_ARGS;
    }

    state = &pdesc->key_states[param->set_param.key_id];
    state->long_time = param->set_param.long_time;
    state->repeat_speed = param->set_param.repeat_speed;
    state->long_count = 0U;
    state->repeat_count = 0U;

    return E_OK;
}

static int32_t _ioctl_set_event_cb(button_describe_t *pdesc, void *args)
{
    union button_ioctl_param *param = (union button_ioctl_param *)args;

    if (param == NULL) {
        return E_WRONG_ARGS;
    }

    pdesc->cb.event = param->set_event_cb.event;
    pdesc->cb.ctx = param->set_event_cb.ctx;

    return E_OK;
}

static void _button_reset_runtime(button_describe_t *pdesc)
{
    uint32_t i = 0U;

    _button_fifo_clear(pdesc);

    for (i = 0U; i < pdesc->number_of_keys; ++i) {
        pdesc->key_states[i].filter_count = 0U;
        pdesc->key_states[i].long_count = 0U;
        pdesc->key_states[i].long_time = pdesc->key_cfgs[i].long_time;
        pdesc->key_states[i].state = 0U;
        pdesc->key_states[i].repeat_speed = pdesc->key_cfgs[i].repeat_speed;
        pdesc->key_states[i].repeat_count = 0U;
    }
}

static int32_t _button_scan_all(button_describe_t *pdesc)
{
    uint32_t i = 0U;

    /* 多键扫描共享状态和 FIFO，锁由移植层按需提供。 */
    if (pdesc->ops.lock != NULL) {
        pdesc->ops.lock(pdesc);
    }

    for (i = 0U; i < pdesc->number_of_keys; ++i) {
        _button_detect_one(pdesc, i);
    }

    if (pdesc->ops.unlock != NULL) {
        pdesc->ops.unlock(pdesc);
    }

    return E_OK;
}

static void _button_detect_one(button_describe_t *pdesc, uint32_t key_id)
{
    const struct button_key_cfg *cfg = NULL;
    struct button_key_state *state = NULL;
    uint16_t filter_time = 0U;
    bool active = false;

    cfg = &pdesc->key_cfgs[key_id];
    state = &pdesc->key_states[key_id];
    filter_time = (pdesc->filter_time == 0U) ? 1U : pdesc->filter_time;
    active = (cfg->is_active != NULL) ? cfg->is_active(cfg->ctx) : false;

    if (active) {
        /* 按下和释放共用一套对称滤波计数，稳定后才改变逻辑状态。 */
        if (state->filter_count < filter_time) {
            state->filter_count = filter_time;
        } else if (state->filter_count < (uint16_t)(2U * filter_time)) {
            state->filter_count++;
        } else {
            if (state->state == 0U) {
                state->state = 1U;
                _button_emit_event(pdesc, BUTTON_EVENT_CODE(key_id, BUTTON_EVENT_DOWN));
            }

            /* 长按和连发只在稳定按下后开始计数。 */
            if (state->long_time > 0U) {
                if (state->long_count < state->long_time) {
                    state->long_count++;
                    if (state->long_count == state->long_time) {
                        state->state = 2U;
                        _button_emit_event(pdesc, BUTTON_EVENT_CODE(key_id, BUTTON_EVENT_LONG));
                    }
                } else if (state->repeat_speed > 0U) {
                    state->repeat_count++;
                    if (state->repeat_count >= state->repeat_speed) {
                        state->repeat_count = 0U;
                        _button_emit_event(pdesc, BUTTON_EVENT_CODE(key_id, BUTTON_EVENT_DOWN));
                    }
                }
            }
        }
    } else {
        if (state->filter_count > filter_time) {
            state->filter_count = filter_time;
        } else if (state->filter_count != 0U) {
            state->filter_count--;
        } else {
            if (state->state == 1U) {
                state->state = 0U;
                _button_emit_event(pdesc, BUTTON_EVENT_CODE(key_id, BUTTON_EVENT_SHORT_UP));
            } else if (state->state == 2U) {
                state->state = 0U;
                _button_emit_event(pdesc, BUTTON_EVENT_CODE(key_id, BUTTON_EVENT_LONG_UP));
            }
        }

        state->long_count = 0U;
        state->repeat_count = 0U;
    }
}

static void _button_emit_event(button_describe_t *pdesc, uint32_t event_code)
{
    /* 事件同时进入 FIFO 和可选回调，兼容轮询与即时通知两种使用方式。 */
    _button_fifo_push(pdesc, event_code);
    if (pdesc->cb.event != NULL) {
        pdesc->cb.event(event_code, pdesc->cb.ctx);
    }
}

static void _button_fifo_push(button_describe_t *pdesc, uint32_t event_code)
{
    uint16_t next = 0U;
    next = (uint16_t)((pdesc->priv.fifo_write + 1U) % pdesc->event_fifo_size);

    if (next == pdesc->priv.fifo_read) {
        pdesc->priv.fifo_read = (uint16_t)((pdesc->priv.fifo_read + 1U) % pdesc->event_fifo_size);
    }

    pdesc->event_fifo[pdesc->priv.fifo_write] = event_code;
    pdesc->priv.fifo_write = next;
}

static bool _button_fifo_pop(button_describe_t *pdesc, uint32_t *event_code)
{
    if (event_code == NULL) {
        return false;
    }

    if (pdesc->priv.fifo_read == pdesc->priv.fifo_write) {
        return false;
    }

    *event_code = pdesc->event_fifo[pdesc->priv.fifo_read];
    pdesc->priv.fifo_read = (uint16_t)((pdesc->priv.fifo_read + 1U) % pdesc->event_fifo_size);

    return true;
}

static void _button_fifo_clear(button_describe_t *pdesc)
{
    uint32_t i = 0U;

    pdesc->priv.fifo_read = 0U;
    pdesc->priv.fifo_write = 0U;

    for (i = 0U; i < pdesc->event_fifo_size; ++i) {
        pdesc->event_fifo[i] = BUTTON_EVENT_NONE_CODE;
    }
}
/*---------- end of file ----------*/
