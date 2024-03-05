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
 *
 * 需要根据知己的SHELL_REC_MAX_SIZE大小修改Heap Size,里面使用了
 * malloc，是从Heap Size申请的，如果空间不够，会Log_e提示报错。
 */

/*---------- includes ----------*/
#define LOG_TAG "shell"

#include "fp_sdk.h"
#include "stdlib.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/
extern void shell_cmd_parser(Shell *shell);
extern void shell_prompt(Shell *shell, uint8_t is_new_line);
extern int  clear(Shell *shell, uint8_t argc, char *argv[]);
/*---------- variable ----------*/

/*---------- function ----------*/

int shell_init(Shell *shell)
{
    assert(shell->shell_write);
    assert(shell->shell_read);

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    extern const unsigned int shell_command$$Base;
    extern const unsigned int shell_command$$Limit;
    shell->commandList.start = (shell_command_t *)(&shell_command$$Base);
    shell->commandList.end   = (shell_command_t *)(&shell_command$$Limit);
    shell->commandList.count = ((size_t)(&shell_command$$Limit) - (size_t)(&shell_command$$Base)) / sizeof(shell_command_t);

#elif defined(__ICCARM__) || defined(__ICCRX__)
#pragma section = "shell_command"
    fp_shell.commandList.base  = (shell_command_t *)(__section_begin("shell_command"));
    fp_shell.commandList.count = ((size_t)(__section_end("shell_command")) - (size_t)(__section_begin("shell_command"))) / sizeof(shell_command_t);
#elif defined(__GNUC__)
    fp_shell.commandList.base  = (shell_command_t *)(&_shell_command_start);
    fp_shell.commandList.count = ((size_t)(&_shell_command_end) - (size_t)(&_shell_command_start)) / sizeof(shell_command_t);
#else
#error not supported compiler, please use command table mode
#endif
    INIT_LIST_HEAD(&shell->parser.buff.list);
    shell->parser.buff.data = 0;// 后面删除逻辑用到这个，必须赋值给空格，否则最后一个字节会删不掉..
    shell->parser.cursor_buff = &shell->parser.buff;
    shell->user.name          = SHELL_DEFAULT_NAME;
    shell->user.name_size     = sizeof(SHELL_DEFAULT_NAME) + 3;
    clear(shell, 1, NULL);
    return 0;
}

void shell_loop(Shell *shell)
{
    assert(shell->shell_read);
    assert(shell->parser.cursor_buff);
    shell->recv_len = shell->shell_read(shell->recv_buf);
    if (shell->recv_len == 0) {
        return;
    }

    shell_cmd_parser(shell);
}

/*---------- end of file ----------*/
