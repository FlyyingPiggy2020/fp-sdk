/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at_cmd_register.h
 * @Author       : lxf
 * @Date         : 2024-11-06 17:33:24
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-11-08 11:56:17
 * @Brief        :
 */

/*---------- includes ----------*/
#include "at_core.h"
#include <stdbool.h>
/*---------- macro ----------*/
typedef struct {
    at_cmd_struct *sub_cmd_list; /*!< List of subcommands */
    uint16_t list_number;
    const char *cmd_name; /*!< Name of the AT command set */
} at_cmd_set_register_t;

/**
 * @brief 注册一个命令组，前缀为cmd的AT指令会从list中寻址。
 * @param {at_cmd_set_register_t} *list 指向命令列表的指针
 * @param {const char} *name 取个名字
 * @param {const char} *cmd AT指令前缀
 * @return {*}
 */
#define AT_CMD_SET_REGISTER_FN(list, name, cmd) \
    __attribute__((used)) __attribute__((section(".at_cmd."))) at_cmd_set_register_t at_cmd_##name = { .sub_cmd_list = list, .list_number = sizeof(list) / sizeof(list[0]), .cmd_name = cmd }

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
