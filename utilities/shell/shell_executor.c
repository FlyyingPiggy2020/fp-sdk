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
 * @FilePath     : shell_executor.c
 * @Author       : lxf
 * @Date         : 2024-03-01 11:52:07
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-02 14:13:10
 * @Brief        : shell命令执行器
 */

/*---------- includes ----------*/

#include "inc/shell.h"
#include "utilities/log/inc/log.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

extern void shell_prompt(Shell *shell, uint8_t is_new_line);
extern void shell_output(Shell *shell, const char *format, ...);
/*---------- variable ----------*/
/*---------- function ----------*/

void shell_executor(Shell *shell, char *data, fp_size_t size)
{
    volatile const shell_command_t *fn_ptr = NULL;

    int argc = 0;

    char *argv[SHELL_REC_MAX_ARGS] = { NULL };

    // strtok会分割*data,让他变得不完整,用空格分割接收到的字符串
    char *p2 = strtok(data, " ");
    while (p2 && argc < SHELL_REC_MAX_ARGS - 1) {
        argv[argc++] = p2;

        p2 = strtok(0, " ");
    }
    if (argc >= SHELL_REC_MAX_ARGS - 1) {
        shell->shell_write("\r\n", 2);
        log_e("args is large than 10.");
        shell_prompt(shell, 0);
        return;
    }
    argv[argc] = NULL;

    if (argv[0] == NULL) { /* 什么都没接收到 */
        shell_prompt(shell, 1);
        return;
    }

    for (fn_ptr = (shell_command_t *)shell->commandList.start; fn_ptr < (shell_command_t *)shell->commandList.end; fn_ptr++) {
        if (!strcmp(argv[0], fn_ptr->cmd.name)) { /* 接收到正确的命令 */
            fn_ptr->cmd.function(shell, argc, argv);
            shell_prompt(shell, 0);
            return;
        }
    }

    /* 没有找到任何命令 */
    shell_output(shell, NEWLINE "\033[31mcant't find \'%s\' as a command\033[0m", argv[0]); // 输出红色的错误提示
    shell_prompt(shell, 1);
}
/*---------- end of file ----------*/
