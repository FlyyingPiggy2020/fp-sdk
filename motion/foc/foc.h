/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc.h
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-02 16:20:00
 * @Brief        : 扁平 FOC 核心接口
 */

#ifndef __FOC_H__
#define __FOC_H__

/*---------- includes ----------*/
#include "foc_math.h"
#include "foc_port.h"
#include "foc_types.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
bool foc_init(struct foc_motor *motor, const struct foc_config *config, const struct foc_port *port, void *port_ctx);
void foc_command_disable(struct foc_motor *motor);
void foc_command_current(struct foc_motor *motor, foc_scalar_t id_pu, foc_scalar_t iq_pu);
void foc_command_speed(struct foc_motor *motor, foc_scalar_t speed_ref);
void foc_command_align(struct foc_motor *motor);
bool foc_run_fast(struct foc_motor *motor);
void foc_run_slow(struct foc_motor *motor, uint32_t now_ms);
/*---------- end of file ----------*/
#endif
