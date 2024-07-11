/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : export.h
 * @Author       : lxf
 * @Date         : 2023-12-28 10:31:44
 * @LastEditors: flyyingpiggy2020 154562451@qq.com
 * @LastEditTime: 2024-03-17 10:06:26
 * @Brief        : export机制(不需要显示调用初始化函数)
 */

#ifndef __EXPORT_H__
#define __EXPORT_H__
/*---------- includes ----------*/

#include "../../common/inc/fp_def.h"
/*---------- macro ----------*/
/*---------- type define ----------*/

typedef int (*init_fn_t)(void);
#if (USE_ESP == 1)
#define INIT_EXPORT(fn, level) fp_used const init_fn_t __fp_init_##fn fp_section("fpi_fn" level) = fn
#else
#define INIT_EXPORT(fn, level) fp_used const init_fn_t __fp_init_##fn fp_section(".fpi_fn." level) = fn
#endif
/**
 * @brief 板级初始化
 * @return {*}
 */
#define INIT_BOARD_EXPORT(fn)     INIT_EXPORT(fn, "1")

/**
 * @brief 设备初始化
 * @return {*}
 */
#define INIT_DEVICE_EXPORT(fn)    INIT_EXPORT(fn, "2")

/**
 * @brief 组件初始化
 * @return {*}
 */
#define INIT_COMPONENT_EXPORT(fn) INIT_EXPORT(fn, "3")

/**
 * @brief 应用初始化
 * @return {*}
 */
#define INIT_APP_EXPORT(fn)       INIT_EXPORT(fn, "4")

/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/

void fp_components_board_init(void);
void fp_components_init(void);
#if (USE_ESP == 1)
void submain(void);
#endif
/*---------- end of file ----------*/
#endif
