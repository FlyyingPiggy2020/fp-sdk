/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : shell_cmd_clear.c
 * @Author       : lxf
 * @Date         : 2024-03-01 11:53:35
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-02 08:42:45
 * @Brief        : shell内置命令clear
 */

/*---------- includes ----------*/

#include "../inc/shell.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

extern void shell_output(Shell *shell, const char *format, ...);
extern void shell_prompt(Shell *shell, uint8_t is_new_line);
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief clear命令
 * @return {*}
 */
int clear(Shell *shell, uint8_t argc, char *argv[])
{
    if (argc == 1) {
        shell_output(shell, "\033[2J\033[1H");
        shell_prompt(shell, 0);
    }
    return 0;
}
SHELL_EXPORT_CMD(clear, clear);
/*---------- end of file ----------*/
