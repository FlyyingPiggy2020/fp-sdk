/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : shell_executor.c
 * @Author       : lxf
 * @Date         : 2024-03-01 11:52:07
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-02 08:42:16
 * @Brief        : shell命令执行器
 */

/*---------- includes ----------*/

#include "inc/shell.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void shell_prompt(Shell *shell, uint8_t is_new_line);
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
    argv[argc] = NULL;

    if (argv[0] == NULL) {
        shell_prompt(shell, 1);
        return;
    }

    for (fn_ptr = (shell_command_t *)shell->commandList.start; fn_ptr < (shell_command_t *)shell->commandList.end; fn_ptr++) {
        if (!strcmp(argv[0], fn_ptr->cmd.name)) {
            fn_ptr->cmd.function(shell, argc, argv);
            return;
        }
    }
}
/*---------- end of file ----------*/
