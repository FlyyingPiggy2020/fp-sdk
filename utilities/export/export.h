/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : export.h
 * @Author       : lxf
 * @Date         : 2023-12-28 10:31:44
 * @LastEditors: Lu Xianfan 154562451@qq.com
 * @LastEditTime: 2025-08-14 13:34:10
 * @Brief        : export机制
 * you need to implent this function:elab_time_ms();
 */

#ifndef __EXPORT_H__
#define __EXPORT_H__
/*---------- includes ----------*/
#include "fp_def.h"
#include "stdint.h"
/*---------- macro ----------*/
#define EXPORT_ID_INIT (0xa5a5a5a5)
#define EXPORT_ID_POLL (0xbeefbeef)
/*---------- type define ----------*/
typedef struct elab_export_poll_data {
    uint32_t timeout_ms;
} elab_export_poll_data_t;

typedef struct elab_export {
    uint32_t magic_head;
    const char *name;
    void *data;
    void *stack;
    void *object;
    void *func;
    uint16_t stack_size;
    uint16_t queue_size;
    uint16_t priority;
    uint16_t level;
    uint32_t period_ms;
    uint32_t temp[6];
    uint32_t magic_tail;
} elab_export_t;

enum fp_export_level {
    EXPORT_BSP = 0,
    EXPORT_IO_DRIVER,
    EXPORT_COMPONENT,
    EXPORT_DEVICE,
    EXPORT_APP,
    EXPORT_TEST,
    EXPORT_MAX,
};

/**
 * @brief  Initialization function exporting macro.
 * @param  _func   The polling function.
 * @param  _level  The export level. See enum elab_export_level.
 * @retval None.
 */
#define INIT_EXPORT(_func, _level)                                         \
    fp_used const elab_export_t init_##_func fp_section("elab_export") = { \
        .name = "init",                                                    \
        .func = (void *)&_func,                                            \
        .level = _level,                                                   \
        .magic_head = EXPORT_ID_INIT,                                      \
        .magic_tail = EXPORT_ID_INIT,                                      \
    }

/**
 * @brief  Poll function exporting macro.
 * @param  _func       The polling function.
 * @param  _period_ms  The polling period in ms.
 * @retval None.
 */
#define POLL_EXPORT(_func, _period_ms)                                \
    static elab_export_poll_data_t poll_##_func##_data = {            \
        .timeout_ms = 0,                                              \
    };                                                                \
    fp_used const elab_export_t poll_##_func fp_section("expoll") = { \
        .name = "poll",                                               \
        .func = &_func,                                               \
        .data = (void *)&poll_##_func##_data,                         \
        .level = EXPORT_MAX,                                          \
        .period_ms = (uint32_t)(_period_ms),                          \
        .magic_head = EXPORT_ID_POLL,                                 \
        .magic_tail = EXPORT_ID_POLL,                                 \
    }

/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/

void fp_run(void);
void fp_tick_inc(void);
/**
 * @brief  highest priority (no other dependencies)
 * @param  _func       The initialization function.
 * @retval None.
 */
#define INIT_BSP_EXPORT(_func)       INIT_EXPORT(_func, EXPORT_BSP)

/**
 * @brief  Initialization function in IO driver layer.
 * @param  _func       The initialization function.
 * @retval None.
 */
#define INIT_IO_DRIVER_EXPORT(_func) INIT_EXPORT(_func, EXPORT_IO_DRIVER)

/**
 * @brief  Initialization function in component layer.
 * @param  _func       The initialization function.
 * @retval None.
 */
#define INIT_COMPONENT_EXPORT(_func) INIT_EXPORT(_func, EXPORT_COMPONENT)

/**
 * @brief  Initialization function in device layer.
 * @param  _func       The initialization function.
 * @retval None.
 */
#define INIT_DEV_EXPORT(_func)       INIT_EXPORT(_func, EXPORT_DEVICE)

/**
 * @brief  Initialization function in appliation layer.
 * @param  _func       The initialization function.
 * @retval None.
 */
#define INIT_APP_EXPORT(_func)       INIT_EXPORT(_func, EXPORT_APP)

/**
 * @brief  Testing function in unit test layer.
 * @param  _func       The initialization function.
 * @retval None.
 */
#define INIT_EXPORT_TEST(_func)      INIT_EXPORT(_func, EXPORT_TEST)
/*---------- end of file ----------*/
#endif
