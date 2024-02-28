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
 * 需要根据知己的SHELL_REC_MAX_SIZE大小修改Heap Size
 */

/*---------- includes ----------*/
#define LOG_TAG "shell"

#include "fp_sdk.h"
#include "stdlib.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/

char recv_data[SHELL_REC_MAX_SIZE]; // 临时接收buf
/*---------- function prototype ----------*/

static void shell_handler(char *data, fp_size_t size);
static bool is_data_too_long(fp_size_t size);
static bool is_data_special(char data);

static void shell_write_string(const char *string);
static void write_prompt(uint8_t newline);

static void shell_cmd_enter(void);

int clear(uint8_t argc, char *argv[]);
/*---------- variable ----------*/
Shell fp_shell;

/*---------- function ----------*/

extern void (*shell_output)(const char *buffer, fp_size_t size);
extern fp_size_t (*shell_input)(char *log);

int shell_init(void)
{
    assert(shell_output);
    assert(shell_input);

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    extern const unsigned int shell_command$$Base;
    extern const unsigned int shell_command$$Limit;
    fp_shell.commandList.start = (shell_command_t *)(&shell_command$$Base);
    fp_shell.commandList.end   = (shell_command_t *)(&shell_command$$Limit);
    fp_shell.commandList.count = ((size_t)(&shell_command$$Limit) - (size_t)(&shell_command$$Base)) / sizeof(shell_command_t);

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
    INIT_LIST_HEAD(&fp_shell.parser.buff.list);
    fp_shell.user.name = SHELL_DEFAULT_NAME;
    clear(0, NULL);
    write_prompt(0);
    return 0;
}

void shell_loop(void)
{
    fp_size_t len = 0;

    if (shell_input == NULL) {
        return;
    }
    len = shell_input(recv_data);
    if (len == 0) {
        return;
    }

    if (is_data_too_long(len) && !is_data_special(recv_data[0])) {
        // 数据太长并且不是特殊的键值
        shell_handler(recv_data, SHELL_REC_MAX_SIZE - fp_shell.parser.length);
        shell_write_string(NEWLINE);
        log_e("input data is larger than max size");
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
            inputbuff_t *temp = malloc(sizeof(inputbuff_t));
            assert(temp);
            temp->data = *(data + i);
            list_add_tail(&temp->list, &fp_shell.parser.buff.list);
            fp_shell.parser.length++;
            shell_output(data + i, 1);
        }
    }
}

/**
 * @brief 按下回车后接收到的数据解析
 * @param {char} *data
 * @param {fp_size_t} size
 * @return {*} 0:不需要换行;1需要换行
 */
static int cmd_parser_handler(char *data, fp_size_t size)
{
    volatile const shell_command_t *fn_ptr = NULL;

    int argc = 0;

    char *argv[SHELL_REC_MAX_ARGS] = { NULL };

    for (fn_ptr = (shell_command_t *)fp_shell.commandList.start; fn_ptr < (shell_command_t *)fp_shell.commandList.end; fn_ptr++) {
        if (!strcmp(data, fn_ptr->cmd.name)) {
            // strtok会分割*data,让他变得不完整,用空格分割接收到的字符串
            char *p2 = strtok(data, " ");
            while (p2 && argc < SHELL_REC_MAX_ARGS - 1) {
                argv[argc++] = p2;

                p2 = strtok(0, " ");
            }
            argv[argc] = NULL;
            return fn_ptr->cmd.function(argc, argv);
        }
    }

    // 回车后没有匹配到命令
    return 1;
}

static void shell_write_string(const char *string)
{
    shell_output(string, strlen(string));
}
/**
 * @brief 判断是否为特殊命令，例如回车，方向键，tab键等
 * @param {char} data
 * @return {*}
 */
static bool is_data_special(char data)
{
    bool retval = false;
    switch (data) {
        case '\r':
        case '\n':
        case '\033':
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
    return (size + fp_shell.parser.length) < SHELL_REC_MAX_SIZE ? false : true;
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
    char cmdbuf[SHELL_REC_MAX_SIZE] = { 0 };

    uint8_t      index       = 0;
    uint8_t      is_new_line = 1;
    inputbuff_t *cmd         = NULL;

    struct list_head *pos;
    struct list_head *pos_tmp;
    // 遍历list,把data提取出来,并释放该节点
    list_for_each_safe(pos, pos_tmp, &fp_shell.parser.buff.list)
    {
        cmd = list_entry(pos, inputbuff_t, list);

        cmdbuf[index++] = cmd->data;

        list_del(&cmd->list);
        free(cmd);
        cmd = NULL;
    }
    is_new_line            = cmd_parser_handler(cmdbuf, fp_shell.parser.length);
    fp_shell.parser.length = 0;
    write_prompt(is_new_line);
}

/**
 * @brief clear命令
 * @return {*}
 */
int clear(uint8_t argc, char *argv[])
{
    shell_write_string("\033[2J\033[1H");
    return 0;
}
SHELL_EXPORT_CMD(clear, clear);
/**
 * @brief ANSI Escape code :https://blog.csdn.net/q1003675852/article/details/134999871
 * @return {*}
 */
void ansi_escape_code(char *data, uint8_t len)
{
    memcpy(data, SHELL_CURSOR_UP, len);
}
/*---------- end of file ----------*/
