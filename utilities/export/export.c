/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : export.c
 * @Author       : lxf
 * @Date         : 2023-12-28 10:31:34
 * @LastEditors: flyyingpiggy2020 154562451@qq.com
 * @LastEditTime: 2024-03-17 10:17:49
 * @Brief        : export机制(不需要显示调用初始化函数)
 */

/*---------- includes ----------*/

#include "inc/export.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief
 * rti_start             --> 0
 * INIT_BOARD_EXPORT     --> 1
 * rti_board_end         --> 1.end
 * INIT_DEVICE_EXPORT    --> 2
 * INIT_COMPONENT_EXPORT --> 3
 * INIT_APP_EXPORT       --> 4
 *
 * rti_end               --> 4.end
 * @return {*}
 */

#if (USE_ESP == 1)
#else
static int fpi_start(void)
{
    return 0;
}
INIT_EXPORT(fpi_start, "0");

static int fpi_board_start(void)
{
    return 0;
}
INIT_EXPORT(fpi_board_start, "0.end");

static int fpi_board_end(void)
{
    return 0;
}
INIT_EXPORT(fpi_board_end, "1.end");

static int fpi_end(void)
{
    return 0;
}
INIT_EXPORT(fpi_end, "4.end");
#endif
/**
 * @brief 板级部分启动
 * @return {*}
 */
void fp_components_board_init(void)
{
    volatile const init_fn_t *fn_ptr;
#if (USE_ESP == 1)
    extern const unsigned int _fpi_fn1_start;
    extern const unsigned int _fpi_fn1_end;
    for (fn_ptr = (void *)(&_fpi_fn1_start); fn_ptr < (void *)(&_fpi_fn1_end); fn_ptr++) {
        (*fn_ptr)();
    }
#else
    for (fn_ptr = &__fp_init_fpi_board_start; fn_ptr < &__fp_init_fpi_board_end; fn_ptr++) {
        (*fn_ptr)();
    }
#endif
}

/**
 * @brief 设备、组件、app依次启动
 * @return {*}
 */
void fp_components_init(void)
{
    volatile const init_fn_t *fn_ptr;
#if (USE_ESP == 1)
    extern const unsigned int _fpi_fn2_start;
    extern const unsigned int _fpi_fn2_end;
    extern const unsigned int _fpi_fn3_start;
    extern const unsigned int _fpi_fn3_end;
    extern const unsigned int _fpi_fn4_start;
    extern const unsigned int _fpi_fn4_end;
    for (fn_ptr = (void *)(&_fpi_fn2_start); fn_ptr < (void *)(&_fpi_fn2_end); fn_ptr++) {
        (*fn_ptr)();
    }
    for (fn_ptr = (void *)(&_fpi_fn3_start); fn_ptr < (void *)(&_fpi_fn3_end); fn_ptr++) {
        (*fn_ptr)();
    }
    for (fn_ptr = (void *)(&_fpi_fn4_start); fn_ptr < (void *)(&_fpi_fn4_end); fn_ptr++) {
        (*fn_ptr)();
    }
#else
    for (fn_ptr = &__fp_init_fpi_board_end; fn_ptr < &__fp_init_fpi_end; fn_ptr++) {
        (*fn_ptr)();
    }
#endif
}

#ifdef __ARMCC_VERSION
void $Sub$$main(void)
{
    extern int $Super$$main(void);
    fp_components_board_init();
    fp_components_init();
    $Super$$main();
}
#endif

#if (USE_ESP == 1)
// your need to add this funciton int to app_mian()
void submain(void)
{
    fp_components_board_init();
    fp_components_init();
}
#endif
/*---------- end of file ----------*/
