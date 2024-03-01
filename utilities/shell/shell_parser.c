/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : shell_parser.c
 * @Author       : lxf
 * @Date         : 2024-03-01 11:51:39
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-01 15:41:52
 * @Brief        : shell命令解析器
 */

/*---------- includes ----------*/

#include "inc/shell.h"
#include "stdlib.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

bool shell_parser_cmd_line_check_ANSI(Shell *shell, char data);

extern void shell_executor(Shell *shell, char *data, fp_size_t size);
/*---------- variable ----------*/
/*---------- function ----------*/

void shell_cmd_parser(Shell *shell)
{
    for (fp_size_t i = 0; i < shell->recv_len; i++) {
        if (shell->recv_buf[i] == '\r' || shell->recv_buf[i] == '\n') { // 回车
            char cmdbuf[SHELL_REC_MAX_SIZE] = { 0 };

            uint8_t index       = 0;
            uint8_t is_new_line = 1;

            inputbuff_t *cmd = NULL;

            struct list_head *pos;
            struct list_head *pos_tmp;

            // 遍历list,把data提取出来,并释放该节点
            list_for_each_safe(pos, pos_tmp, &shell->parser.buff.list)
            {
                cmd = list_entry(pos, inputbuff_t, list);

                cmdbuf[index++] = cmd->data;

                list_del(&cmd->list);
                free(cmd);
                cmd = NULL;
            }

            shell_executor(shell, cmdbuf, shell->parser.length); // 解析函数
            shell->parser.length = 0;
            return;
        }

        if (shell_parser_cmd_line_check_ANSI(shell, shell->recv_buf[i])) {
            continue;
        }

        if (shell->parser.length >= SHELL_REC_MAX_SIZE - 1) {
            log_e("recv data too long.");
            return;
        }
        
        inputbuff_t *temp = malloc(sizeof(inputbuff_t));
        if (temp == NULL) {
            log_e("malloc memory failed");
        } else {
            temp->data = shell->recv_buf[i];
            list_add_tail(&temp->list, &shell->parser.buff.list);
            shell->parser.length++;
            shell->shell_write(shell->recv_buf + i, 1);
        }
    }
}

/**
 * @brief 检测 ANSI Escape code
 * @param {char} data
 * @return {*}
 */
bool shell_parser_cmd_line_check_ANSI(Shell *shell, char data)
{
    typedef enum {
        NORMAL = 0,
        ESC,
        MULTI_KEY,
    } keytype_e;

    if (data == '\033') { // ESC
        shell->parser.key_type = ESC;
        return true;
    } else if (data == '[') { // CSI
        if (shell->parser.key_type == ESC) {
            shell->parser.key_type = MULTI_KEY;
        }
        return true;
    } else if (data == 'A') { // UP
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
        }
        return true;
    } else if (data == 'B') { // DOWN
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
        }
        return true;
    } else if (data == 'C') { // LEFT
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
        }
        return true;
    } else if (data == 'D') { // RIGHT
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
        }
        return true;
    }
    return false;
}

/*---------- end of file ----------*/
