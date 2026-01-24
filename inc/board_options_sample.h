/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : board_options.h
 * @Author       : lxf
 * @Date         : 2025-12-12 10:12:42
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-13 09:11:12
 * @Brief        : 一份用户配置文件示例模版
 * 可以复制一份，并且改名为board_options.h
 * PS: Keil mdk (目前版本v5.43为止)，它实现的 wizard configuration有bug，无法正常解析中文文档。并且部分语法不支持
 * 请使用VScode中的Arm CMSIS Solution插件作为解析该配置的方式。或者手动修改。
 * https://open-cmsis-pack.github.io/Open-CMSIS-Pack-Spec/main/html/configWizard.html#configWizard_display
 */
#ifndef __BOARD_OPTIONS_H__
#define __BOARD_OPTIONS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "cpu.h"
/*---------- macro ----------*/
/**
 * @brief 系统API接口 __前缀代表不直接调用(具体函数在cpu.c中实现)
 * @return {*}
 */
#define __delay_ms(ms)              (mdelay(ms))
#define __delay_us(us)              (udelay(us))
#define __get_ticks()               (tick_get())
#define __get_ticks_from_isr()      (tick_get_from_isr())
#define __reset_system()            cpu_reset()
#define __enter_critical()          disable_irq()
#define __exit_critical()           enable_irq()
#define __enter_critical_from_isr() disable_irq()
#define __exit_critical_from_isr()  enable_irq()
#define __ticks2ms(ticks)           (ticks)
#define __ms2ticks(ms)              (ms)
#define __malloc(size)              __heap_malloc(size)
#define __free(ptr)                 __heap_free(ptr)
//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <h> FP-SDK功能配置
// =======================
//
//   <e> 日志功能是否开启
//   <i> Default: 日志开启
#define CONFIG_FPLOG                1
// </e>

//   <e> 动态内存管理
//   <i> Default: 内存管理开启
//   <o> 动态内存管理算法
//      <0=> 用户自定义
//      <1=> heap4(移植自FreeRTOS)
//      <i> Default: heap4
#define CONFIG_HEAP_TYPE            1
//	 <o> 动态内存池大小 <f.d>
#define CONFIG_HEAP_TOTAL_SIZE      10240
// </e>

// </h>
// <h>
// </h>
//------------- <<< end of configuration section >>> -----------------------
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
