/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_cfg.h
 * @Author       : Codex
 * @Date         : 2026-03-21 10:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 13:20:58
 * @Brief        : FOC 框架编译期配置
 */

#ifndef __FOC_CFG_H__
#define __FOC_CFG_H__

/*---------- includes ----------*/
/*---------- macro ----------*/
/*
 * 相电流采样拓扑
 */
#ifndef FOC_CURRENT_SENSE_MODE
#define FOC_CURRENT_SENSE_MODE 2 /* 1=单电阻采样, 2=双电阻采样, 3=三电阻采样 */
#endif

#if (FOC_CURRENT_SENSE_MODE == 1)
#error "NOT SUPPORT THIS SHUNT CURRENT SENSE"
#elif (FOC_CURRENT_SENSE_MODE == 2)
#elif (FOC_CURRENT_SENSE_MODE == 3)
#error "NOT SUPPORT THIS SHUNT CURRENT SENSE"
#else
#error "FOC_CURRENT_SENSE_MODE invalid"
#endif

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif /* __FOC_CFG_H__ */
