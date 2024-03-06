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
 * @FilePath     : shell_parser.c
 * @Author       : lxf
 * @Date         : 2024-03-01 11:51:39
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-05 17:19:04
 * @Brief        : shell命令解析器
 */

/*---------- includes ----------*/

#include "inc/shell.h"
#include "utilities/log/inc/log.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

static fp_size_t shell_parser_read_clean_list(Shell *shell, char *recv_buf);
static fp_size_t shell_parser_read_list(Shell *shell, char *recv_buf);

static void shell_parser_backspace(Shell *shell);
static void shell_parser_insert_data_to_cursor_postion(Shell *shell, char buf);
static void shell_parser_tab(Shell *shell, char *data, fp_size_t size);

bool shell_parser_cmd_line_check_ANSI(Shell *shell, char data);

extern void shell_executor(Shell *shell, char *data, fp_size_t size);
extern void shell_output(Shell *shell, const char *format, ...);
extern void shell_prompt(Shell *shell, uint8_t is_new_line);
extern void shell_prompt_printf_from_cursor(Shell *shell);
extern void shell_prompt_del_from_cursor(Shell *shell);
extern void shell_prompt_printf_without_cursor(Shell *shell, uint8_t is_cursor_add);
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief 键值解析器
 * @param {Shell} *shell
 * @return {*}
 */
void shell_cmd_parser(Shell *shell)
{
    char recv_buf[SHELL_REC_MAX_SIZE] = { 0 };

    fp_size_t recv_len;

    for (fp_size_t i = 0; i < shell->recv_len; i++) {
        if (shell->recv_buf[i] == '\r' || shell->recv_buf[i] == '\n') { // 回车
            recv_len = shell_parser_read_clean_list(shell, recv_buf);
            shell_executor(shell, recv_buf, recv_len); // 解析函数
            return;
        }
        if (shell->recv_buf[i] == SHELL_TAB) {
            recv_len = shell_parser_read_list(shell, recv_buf);
            shell_parser_tab(shell, recv_buf, recv_len);
            return;
        }

        if (shell_parser_cmd_line_check_ANSI(shell, shell->recv_buf[i])) { // 特殊键值
            continue;
        }

        if (shell->parser.length >= SHELL_REC_MAX_SIZE - 1) { // 判断长度是否超出了Recv buff
            shell_parser_read_clean_list(shell, recv_buf);
            log_e("recv data too long.");
            shell_prompt(shell, 1);
            return;
        }

        // 如果是普通的键值
        shell_parser_insert_data_to_cursor_postion(shell, shell->recv_buf[i]);
    }
}
/**
 * @brief 将对应的buff插入光标位置(让光标后的内容后移)
 * @param {Shell} *shell
 * @param {char} buf
 * @return {*}
 */
static void shell_parser_insert_data_to_cursor_postion(Shell *shell, char buf)
{
    // 1.数据结构中插入。
    inputbuff_t *temp = malloc(sizeof(inputbuff_t));

    if (temp == NULL) {
        log_e("malloc memory failed");
        return;
    } else {
        temp->data = buf;
        list_add(&temp->list, &shell->parser.cursor_buff->list);
        shell->parser.cursor_buff = temp;
        shell->parser.length++;
        shell->parser.cursor++;

        // 2.在终端显示上插入。
        shell_prompt_printf_from_cursor(shell);
    }
}

/**
 * @brief 读取输入的字节，并且释放这个链表
 * @param {Shell} *shell
 * @param {char} *recv_buf 读取到这个buff
 * @return {*} 返回实际读取的size
 */
static fp_size_t shell_parser_read_clean_list(Shell *shell, char *recv_buf)
{
    uint8_t index       = 0;
    uint8_t is_new_line = 1;

    inputbuff_t *cmd = NULL;

    struct list_head *pos;
    struct list_head *pos_tmp;

    // 遍历list,把data提取出来,并释放该节点
    list_for_each_safe(pos, pos_tmp, &shell->parser.buff.list)
    {
        cmd = list_entry(pos, inputbuff_t, list);

        recv_buf[index++] = cmd->data;

        list_del(&cmd->list);
        if (cmd != NULL) {
            free(cmd);
            cmd = NULL;
        }
    }

    shell->parser.length      = 0;
    shell->parser.cursor      = 0;
    shell->parser.cursor_buff = &shell->parser.buff;
    return index;
}

/**
 * @brief 读取但是不清除队列
 * @param {Shell} *shell
 * @param {char} *recv_buf
 * @return {*}
 */
static fp_size_t shell_parser_read_list(Shell *shell, char *recv_buf)
{
    uint8_t index       = 0;
    uint8_t is_new_line = 1;

    inputbuff_t *cmd = NULL;

    struct list_head *pos;
    struct list_head *pos_tmp;

    // 遍历list,把data提取出来,并释放该节点
    list_for_each_safe(pos, pos_tmp, &shell->parser.buff.list)
    {
        cmd = list_entry(pos, inputbuff_t, list);

        recv_buf[index++] = cmd->data;

        cmd = NULL;
    }
    return index;
}
/**
 * @brief 删除光标处的字节
 * @param {Shell} *shell
 * @return {*}
 */
static void shell_parser_clean_cursor_postion_data(Shell *shell)
{

    inputbuff_t *temp = NULL;

    temp = shell->parser.cursor_buff;

    shell->parser.cursor_buff = list_prev_entry(temp, inputbuff_t, list);

    // 数据结构中删除

    list_del(&temp->list);
    if (temp != NULL) {
        free(temp);
        temp = NULL;
    }
    shell->parser.cursor--;
    shell->parser.length--;
    // 终端显示上删除
    shell_prompt_del_from_cursor(shell);
}
/**
 * @brief 检测 ANSI Escape code
 * https://en.wikipedia.org/wiki/ANSI_escape_code#Terminal_input_sequences
 * @param {char} data
 * @return {*}
 */
bool shell_parser_cmd_line_check_ANSI(Shell *shell, char data)
{
    typedef enum {
        NORMAL = 0,
        ESC,
        MULTI_KEY,
        MULTI_KEY1,
    } keytype_e;

    if (data == '\033') { // ESC
        shell->parser.key_type = ESC;
        return true;
    } else if (data == '[') { // CSI
        if (shell->parser.key_type == ESC) {
            shell->parser.key_type = MULTI_KEY;
            return true;
        }

    } else if (data == 'A') { // UP
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
            return true;
        }
    } else if (data == 'B') { // DOWN
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
            return true;
        }
    } else if (data == 'D') { // LEFT
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
            if (shell->parser.cursor > 0) {
                shell_output(shell, "\b");
                shell->parser.cursor--;
                shell->parser.cursor_buff = list_prev_entry(shell->parser.cursor_buff, inputbuff_t, list);
            }
            return true;
        }
    } else if (data == 'C') { // RIGHT
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type = NORMAL;
            if (shell->parser.cursor < shell->parser.length) {
                shell_prompt_printf_without_cursor(shell, 1);
                shell->parser.cursor++;
                shell->parser.cursor_buff = list_next_entry(shell->parser.cursor_buff, inputbuff_t, list);
            }
            return true;
        }
    } else if (data == '\b') { // BACKSPACE
        shell_parser_backspace(shell);
        return true;
    } else if (data >= '1' && data <= '9') {
        if (shell->parser.key_type == MULTI_KEY) {
            shell->parser.key_type        = MULTI_KEY1;
            shell->parser.vt_sequences[0] = data;
            return true;
        }
    } else if (data == '~') {
        // 终止符(详见vt sequences)这部分内容不实现了，主要是可以识别
        // F1,F2这种键值，我这里暂时用不到
        if (shell->parser.key_type == MULTI_KEY1) {
            return true;
        }
    }

    shell->parser.key_type = NORMAL;
    return false;
}

/**
 * @brief shell解析退格键
 * @param {Shell} *shell
 * @return {*}
 */
static void shell_parser_backspace(Shell *shell)
{

    if (shell->parser.cursor > 0) {
        // 删除光标位置的内容
        shell_parser_clean_cursor_postion_data(shell);
    }
}

/**
 * @brief 找到公共字符串
 * @param {char} *strings
 * @param {int} numStrings : 必须大于等于2
 * @return {*}
 */
int findCommonCharacters(const char *strs[], int n, char *com_strs)
{
    if (strs == NULL || n <= 1) {
        return -1;
    }

    uint8_t strs_index = 0; // 第几个字符串
    uint8_t byte_index = 0; // 字符串第几个字节

    while (1) {
        if (*(strs[0] + byte_index) == 0 || byte_index >= SHELL_CMD_MAX_LENS) { // 字符串到'\0'了,或者大于最大匹配长度
            break;
        }
        if (strs_index == n) { // 完成对某个byte的完全匹配
            com_strs[byte_index] = *(strs[0] + byte_index);
            strs_index           = 0;
            byte_index++;
        }
        if (*(strs[0] + byte_index) != *(strs[strs_index] + byte_index)) {
            break;
        }
        strs_index++;
    }

    if (byte_index >= SHELL_CMD_MAX_LENS) {
        return -1;
    }
    return 1;
}

/**
 * @brief 解析tab键位
 * @param {Shell} *shell
 * @return {*}
 */
static void shell_parser_tab(Shell *shell, char *data, fp_size_t size)
{
    uint8_t strings_len = 0;

    const char **strings = NULL; // 动态数组,用realloc申请，保存指针

    char comm_str[SHELL_CMD_MAX_LENS] = { 0 };

    uint16_t cmd_cnt = 0;

    if (size == 0) {

        return;
    }

    for (const shell_command_t *fn_ptr = (shell_command_t *)shell->commandList.start; fn_ptr < (shell_command_t *)shell->commandList.end; fn_ptr++) {
        if (!strncmp(data, fn_ptr->cmd.name, size)) { /* 匹配到到正确的命令 */
            strings = realloc(strings, sizeof(char *) * (++strings_len));
            if (strings != NULL) {
                strings[strings_len - 1] = fn_ptr->cmd.name;
                cmd_cnt++;
            } else {
                if (strings != NULL) {
                    free(strings);
                    strings = NULL;
                }
                log_e("realloc failed, don't have enough memory");
            }
        }
    }

    if (cmd_cnt == 0) {
        // do nothing;
    } else if (cmd_cnt == 1) {
        // 自动补全
        for (uint8_t i = 0; i < strlen(strings[0]); i++) {
            if (*(strings[0] + i) == *(data + i)) {
                continue;
            }
            shell_parser_insert_data_to_cursor_postion(shell, *(strings[0] + i));
        }
    } else {
        if (findCommonCharacters(strings, strings_len, comm_str)) {
            // 匹配公共词缀
            for (uint8_t i = 0; i < cmd_cnt; i++) {
                shell_output(shell, "\r\n%s", strings[i]);
            }
            shell_prompt(shell, 1);
            shell_output(shell, "%s", data);
            for (uint8_t i = 0; i < strlen(comm_str); i++) {
                if (*(comm_str + i) == *(data + i)) {
                    continue;
                }
                shell_parser_insert_data_to_cursor_postion(shell, *(comm_str + i));
            }
        }
    }

    if (strings != NULL) {
        free(strings);
        strings = NULL;
    }
}

/*---------- end of file ----------*/
