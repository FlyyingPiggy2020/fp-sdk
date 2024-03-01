/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : shell_prompt.c
 * @Author       : lxf
 * @Date         : 2024-03-01 16:05:10
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-02 08:41:23
 * @Brief        :
 */

/*---------- includes ----------*/

#include "inc/shell.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/

static char shell_output_buf[SHELL_TRANS_MAX_SIZE] = { 0 };

/*---------- function ----------*/

/**
 * @brief shell格式化输出
 * @param {Shell} *shell
 * @param {char} *format
 * @return {*}
 */
void shell_output(Shell *shell, const char *format, ...)
{
    int fmt_result = 0;
    int output_len = 0;

    va_list args;
    va_start(args, format);

    fmt_result = vsnprintf(shell_output_buf, SHELL_TRANS_MAX_SIZE, format, args);

    if (fmt_result <= SHELL_TRANS_MAX_SIZE && fmt_result > -1) {
        output_len = fmt_result;
    } else {
        fmt_result = SHELL_TRANS_MAX_SIZE;
    }

    if (shell->shell_write != NULL) {
        shell->shell_write(shell_output_buf, output_len);
    }
}

/**
 * @brief 输出用户名
 * @param {Shell} *shell
 * @return {*}
 */
void shell_prompt(Shell *shell, uint8_t is_new_line)
{
    if (is_new_line) {
        shell_output(shell, NEWLINE "%s:$ ", shell->user.name);
    } else {
        shell_output(shell, "%s:$ ", shell->user.name);
    }
}
/*---------- end of file ----------*/
