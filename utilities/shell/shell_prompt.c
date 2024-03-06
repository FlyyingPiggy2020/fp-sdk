/*MIT License

Copyright (c) 2023 Lu Xianfan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : shell_prompt.c
 * @Author       : lxf
 * @Date         : 2024-03-01 16:05:10
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-05 11:43:45
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

/**
 * @brief 打印出光标位置以及光标后面的内容。
 * @param {Shell} *shell
 * @return {*}
 */
void shell_prompt_printf_from_cursor(Shell *shell)
{
    uint8_t index = 0;

    inputbuff_t *temp = NULL;

    char recv_buf[SHELL_REC_MAX_SIZE] = { 0 };

    temp = shell->parser.cursor_buff;
    list_for_each_entry_from(temp, inputbuff_t, &shell->parser.buff.list, list)
    {
        recv_buf[index++] = temp->data;
    }
    shell->shell_write(recv_buf, index);

    // 将光标移动到记录的位置,不采用\033[%dG是因为考虑到了终端会换行,但是单片机又无法知道它会换行。
    for (uint8_t i = shell->parser.cursor; i < shell->parser.length; i++) {
        shell->shell_write("\b", 1);
    }
}

/**
 * @brief 打印出光标后面的内容,但是不打印出光标位置内容
 * @param {Shell} *shell
 * @param {uint8_t} is_cursor_add 是否打印出光标内容
 * @return {*}
 */
void shell_prompt_printf_without_cursor(Shell *shell, uint8_t is_cursor_add)
{
    uint8_t index = 0;

    inputbuff_t *temp = NULL;

    char recv_buf[SHELL_REC_MAX_SIZE] = { 0 };

    temp = shell->parser.cursor_buff;
    if (shell->parser.cursor == 0) { // 当前光标是第一个字节
        list_for_each_entry(temp, inputbuff_t, &shell->parser.buff.list, list)
        {
            recv_buf[index++] = temp->data;
        }
        shell->shell_write(recv_buf, index);
        shell->shell_write(" ", 1);
        // 将光标移动到记录的位置,不采用\033[%dG是因为考虑到了终端会换行,但是单片机又无法知道它会换行。
        for (uint8_t i = shell->parser.cursor; i < shell->parser.length; i++) {
            shell->shell_write("\b", 1);
        }

    } else { // 当前光标不是第一个字节
        list_for_each_entry_from(temp, inputbuff_t, &shell->parser.buff.list, list)
        {
            recv_buf[index++] = temp->data;
        }
        shell->shell_write(recv_buf + 1, index - 1);
        shell->shell_write(" ", 1);
        // 将光标移动到记录的位置,不采用\033[%dG是因为考虑到了终端会换行,但是单片机又无法知道它会换行。
        for (uint8_t i = shell->parser.cursor; i < shell->parser.length; i++) {
            shell->shell_write("\b", 1);
        }
    }
    if (!is_cursor_add) {
        shell->shell_write("\b", 1);
    }
}
/**
 * @brief 删除光标前一个字节的内容
 * @param {Shell} *shell
 * @return {*}
 */
void shell_prompt_del_from_cursor(Shell *shell)
{
    shell->shell_write("\b", 1);
    uint8_t index = 0;

    inputbuff_t *temp = NULL;

    temp = shell->parser.cursor_buff;

    // 光标没被移动过
    if (shell->parser.cursor == shell->parser.length) {
        shell->shell_write(" ", 1);
        shell->shell_write("\b", 1);
    } else {
        shell_prompt_printf_without_cursor(shell, 0);
    }
}

/*---------- end of file ----------*/
