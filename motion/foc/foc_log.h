/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_log.h
 * @Author       : Codex
 * @Date         : 2026-03-17 16:35:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17 16:35:00
 * @Brief        : FOC 模块日志宏定义
 */

#ifndef __FOC_LOG_H__
#define __FOC_LOG_H__

/*---------- includes ----------*/
/*---------- macro ----------*/
/*
 * FOC 核心层不直接依赖任何日志框架。
 * 如果外部项目需要接日志，只需在编译选项或公共头文件中重定义这些宏即可。
 */
#ifndef FOC_LOG_ERROR
#define FOC_LOG_ERROR(fmt, ...)
#endif

#ifndef FOC_LOG_WARN
#define FOC_LOG_WARN(fmt, ...)
#endif

#ifndef FOC_LOG_INFO
#define FOC_LOG_INFO(fmt, ...)
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
