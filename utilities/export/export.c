/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : export.c
 * @Author       : lxf
 * @Date         : 2023-12-28 10:31:34
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-22 15:19:39
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

/**
 * @brief 板级部分启动
 * @return {*}
 */
void fp_components_board_init(void)
{
    volatile const init_fn_t *fn_ptr;
    for (fn_ptr = &__fp_init_fpi_board_start; fn_ptr < &__fp_init_fpi_board_end; fn_ptr++) {
        (*fn_ptr)();
    }
}

/**
 * @brief 设备、组件、app依次启动
 * @return {*}
 */
void fp_components_init(void)
{
    volatile const init_fn_t *fn_ptr;
    for (fn_ptr = &__fp_init_fpi_board_end; fn_ptr < &__fp_init_fpi_end; fn_ptr++) {
        (*fn_ptr)();
    }
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
/*---------- end of file ----------*/
