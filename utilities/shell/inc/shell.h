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
 * @FilePath     : shell.h
 * @Author       : lxf
 * @Date         : 2024-02-23 14:33:31
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-23 14:33:32
 * @Brief        : shell (Test by MobaXterm)
 */

#ifndef __SHELL_H__
#define __SHELL_H__
/*---------- includes ----------*/

#include "stdarg.h"
#include "stdlib.h"
#include "string.h"

#include "utilities/clists/inc/clists.h"
#include "utilities/common/inc/fp_def.h"

/*---------- macro ----------*/
#define SHELL_REC_MAX_SIZE   128 // shell最大接收大小
#define SHELL_TRANS_MAX_SIZE 128 // shell最大输出大小
#define SHELL_REC_MAX_ARGS   10  // 最大变量个数
#define SHELL_CMD_MAX_LENS   50  // 命令的最大长度：因为tab补全的时候会用到。relloc的话需要在函数外面free感觉不太好。

#define SHELL_DEFAULT_NAME   "fpshell"
#define NEWLINE              "\r\n"
/* https://www.asciitable.com/ */
/* https://blog.csdn.net/q1003675852/article/details/134999871 */

#define SHELL_TAB            0X09

/*---------- type define ----------*/

typedef struct inputbuff {

    uint8_t          data; // 数据
    struct list_head list; // 链表
} inputbuff_t;
/**
 * @brief shell定义
 */
typedef struct shell_def {
    char    recv_buf[SHELL_REC_MAX_SIZE];
    uint8_t recv_len;
    struct {
        uint8_t      length;          // 当前输入数据长度
        uint8_t      cursor;          // 当前光标位置(相对于user.name)
        inputbuff_t  buff;            // 输入buff
        inputbuff_t *cursor_buff;     // 当前光标位置的buff指针
        uint8_t      key_type;        // 判断特殊键值
        uint8_t      vt_sequences[1]; // vt模式的特殊键值缓存
    } parser;

    struct {
        void   *start; // 命令表基地址
        void   *end;
        uint8_t count; // 命令数量
    } commandList;

    struct {
        const char *name; // 名字
        uint8_t     name_size;
    } user;

    void (*shell_write)(const char *send_buf, fp_size_t size);
    fp_size_t (*shell_read)(char *recv_buf);
} Shell;

typedef struct shell_command {
    struct
    {
        const char *name;
        int (*function)(Shell *, uint8_t argc, char *argv[]);
    } cmd;

} shell_command_t;

#define SHELL_EXPORT_CMD(_name, _func)                                                  \
    const char shell_cmd_##_name[] = #_name;                                            \
                                                                                        \
    fp_used const shell_command_t shell_command_##_name fp_section("shell_command") = { \
        .cmd.name     = shell_cmd_##_name,                                              \
        .cmd.function = (int (*)(Shell *, uint8_t, char **))_func,                      \
    }

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
int  shell_init(Shell *shell);
void shell_loop(Shell *shell);

/*---------- end of file ----------*/
#endif
