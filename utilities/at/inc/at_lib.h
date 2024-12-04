/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at_lib.h
 * @Author       : lxf
 * @Date         : 2024-11-08 08:31:50
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-11-08 13:33:22
 * @Brief        :
 */

#pragma once
/*---------- includes ----------*/
#include "at_core.h"
#include <stdint.h>
/*---------- macro ----------*/

/**
 * @brief AT设置指令参数最大个数
 * @return {*}
 */
#define AT_SET_MAX_SIZE 10
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
at_error_code at_cmd_parser(uint8_t *recv_buf, uint16_t recv_len);
/*---------- end of file ----------*/
