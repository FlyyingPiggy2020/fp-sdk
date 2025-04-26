/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : export.c
 * @Author       : lxf
 * @Date         : 2023-12-28 10:31:34
 * @LastEditors: flyyingpiggy2020 154562451@qq.com
 * @LastEditTime: 2024-03-17 10:17:49
 * @Brief        : export机制
 * origin file from https://gitee.com/event-os/elab/blob/master/elab/common/elab_export.c
 */

/*---------- includes ----------*/
#include "stdlib.h"
#include "inc/export.h"
/*---------- macro ----------*/
#define ELAB_POLL_PERIOD_MAX (2592000000) /* 30 days */
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static uint32_t sys_time_ms = 0;
/*---------- function prototype ----------*/
static void module_null_init(void);
/*---------- variable ----------*/
INIT_BSP_EXPORT(module_null_init);
POLL_EXPORT(module_null_init, (1000 * 60 * 60));

static const uint32_t export_id_table[EXPORT_MAX + 1] = {
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_POLL,
};

static elab_export_t *export_init_table = NULL;
static elab_export_t *export_poll_table = NULL;

/*---------- function ----------*/
/**
 * @brief  SysTick ISR function.
 * @retval None
 */
uint32_t elab_time_ms(void)
{
    return sys_time_ms;
}

static elab_export_t *_get_export_table(uint8_t level)
{
    elab_export_t *func_block = level < EXPORT_MAX ? ((elab_export_t *)&init_module_null_init) : ((elab_export_t *)&poll_module_null_init);

    while (1) {
        uint32_t address_last = ((uint32_t)func_block - sizeof(elab_export_t));
        elab_export_t *table = (elab_export_t *)address_last;
        if (table->magic_head != export_id_table[level] || table->magic_tail != export_id_table[level]) {
            break;
        }
        func_block = table;
    }

    return func_block;
}

/**
 * @brief  eLab exporting function executing.
 * @param  level export level.
 * @retval None
 */
static void _export_func_execute(uint8_t level)
{
    uint32_t export_id = export_id_table[level];

    /* Get the start address of exported poll table. */
    if (export_init_table == NULL) {
        export_init_table = _get_export_table(EXPORT_BSP);
    }
    if (export_poll_table == NULL) {
        export_poll_table = _get_export_table(EXPORT_MAX);
    }

    /* Execute the poll function in the specific level. */
    elab_export_t *export_table = level < EXPORT_MAX ? export_init_table : export_poll_table;
    for (uint32_t i = 0;; i++) {
        if (export_table[i].magic_head == export_id && export_table[i].magic_tail == export_id) {
            if (export_table[i].level == level && level <= EXPORT_APP) {
                ((void (*)(void))export_table[i].func)();
            }

            else if (export_table[i].level == level && level == EXPORT_MAX) {
                elab_export_poll_data_t *data = export_table[i].data;
                // 考虑溢出
                uint32_t _time = elab_time_ms();
                if (((_time >= data->timeout_ms) && (_time - data->timeout_ms < ELAB_POLL_PERIOD_MAX)) || ((_time < data->timeout_ms) && (data->timeout_ms - _time > ELAB_POLL_PERIOD_MAX))) {
                    data->timeout_ms += export_poll_table[i].period_ms;
                    ((void (*)(void))export_poll_table[i].func)();
                }
            }
        } else {
            break;
        }
    }
}

void fp_run(void)
{
    for (uint8_t level = EXPORT_BSP; level <= EXPORT_APP; level++) {
        _export_func_execute(level);
    }

    while (1) {
        _export_func_execute(EXPORT_MAX);
    }
}

void fp_tick_inc(void)
{
    sys_time_ms++;
}

static void module_null_init(void)
{
    /* NULL */
}
/*---------- end of file ----------*/
