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
 * @FilePath     : shell.c
 * @Author       : lxf
 * @Date         : 2024-02-23 14:33:11
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-23 14:35:40
 * @Brief        : shell (Test by MobaXterm)
 */

/*---------- includes ----------*/
#define LOG_TAG "shell"
#include "fp_sdk.h"

/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/

char recv_data[SHELL_REC_MAX_SIZE]; // 临时接收buf
/*---------- function prototype ----------*/

static void shell_handler(char *data, fp_size_t size);
static bool is_data_too_long(fp_size_t size);
static bool is_data_special(char data);

static void write_prompt(uint8_t newline);

static void shell_cmd_enter(void);
/*---------- variable ----------*/
Shell fp_shell;
/*---------- function ----------*/

extern void (*shell_output)(const char *buffer, fp_size_t size);
extern fp_size_t (*shell_input)(char *log);

int shell_init(void)
{
    assert(shell_output);
    assert(shell_input);
    //    INIT_LIST_HEAD(fp_shell.parser.buff.list);

    fp_shell.user.name = SHELL_DEFAULT_NAME;
    write_prompt(1);
    return 0;
}

void shell_loop(void)
{
    fp_size_t len = 0;

    if (shell_input == NULL) {
        return;
    }
    len = shell_input(recv_data);
    if (len != 0 && is_data_too_long(len)) {
        // 数据太长
        shell_handler(recv_data, SHELL_REC_MAX_SIZE - fp_shell.parser.buffindex);
    } else {
        shell_handler(recv_data, len);
    }
}

static void shell_handler(char *data, fp_size_t size)
{
    for (fp_size_t i = 0; i < size; i++) {
        if (is_data_special(*(data + i))) { // 特殊命令
            if (*(data + i) == '\r' || *(data + i) == '\n') {
                shell_cmd_enter(); // 回车
            }
        } else {
            // 非特殊命令
            shell_output(data + i, 1);
        }
    }
}

static void shell_write_string(const char *string)
{
    shell_output(string, strlen(string));
}
/**
 * @brief 判断是否为特殊命令
 * @param {char} data
 * @return {*}
 */
static bool is_data_special(char data)
{
    bool retval = false;
    switch (data) {
        case '\r':
        case '\n':
            retval = true;
            break;
        default:
            break;
    }

    return retval;
}

/**
 * @brief 判断是否数据超出限制
 * @param {fp_size_t} size
 * @return {*}
 */
static bool is_data_too_long(fp_size_t size)
{
    return (size + fp_shell.parser.buffindex) < SHELL_REC_MAX_SIZE ? true : false;
}

/**
 * @brief 写命令提示行
 * @param {uint8_t} newline
 * @return {*}
 */
static void write_prompt(uint8_t newline)
{
    if (newline) {
        shell_write_string(NEWLINE);
    }
    shell_write_string(fp_shell.user.name);
    shell_write_string(":");
    shell_write_string("$ ");
}
/**
 * @brief 回车命令
 * @return {*}
 */
static void shell_cmd_enter(void)
{
    write_prompt(1);
}
/**
 * @brief ANSI Escape code :https://blog.csdn.net/q1003675852/article/details/134999871
 * @return {*}
 */
void ansi_escape_code(char *data, uint8_t len)
{
    memcpy(data, SHELL_CURSOR_UP, len);
}
/*---------- end of file ----------*/
