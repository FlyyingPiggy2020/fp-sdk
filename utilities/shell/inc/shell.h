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

#include "fp_sdk.h"

/*---------- macro ----------*/
#define SHELL_REC_MAX_SIZE 128 // shell最大接收大小

#define SHELL_DEFAULT_NAME "fpshell\0"
#define NEWLINE            "\r\n"
/* https://www.asciitable.com/ */
/* https://blog.csdn.net/q1003675852/article/details/134999871 */

#define SHELL_CSI          "\033["
#define SHELL_CURSOR_UP    SHELL_CSI "A"
#define SHELL_TAB          0X09

/*---------- type define ----------*/

// // 用树来保存cli命令
// typedef struct tree_bro { // 树的兄弟
//     struct tree_bro *next;
// } Bro_t;

// typedef struct tree_node {
//     char              *data;        // 节点数据
//     struct tree_node  *parent;      // 家长
//     struct tree_node **child;       // 孩子动态数据
//     int                child_count; // 孩子个数
//     Bro_t              bro_list;    // 兄弟节点
// };

typedef struct inputbuff {

    uint8_t data; // 数据

    struct list_head list; // 链表
} inputbuff_t;
/**
 * @brief shell定义
 */
typedef struct shell_def {
    struct {
        uint8_t     length;    // 当前输入数据长度
        uint8_t     cursor;    // 当前光标位置
        inputbuff_t buff;      // 输入buff
        uint8_t     buffindex; // 输入buff大小
    } parser;

    struct {
        void   *base;  // 命令表基地址
        uint8_t count; // 命令数量
    } commandList;

    struct {
        const char *name; // 名字
    } user;

} Shell;

/**
 * @brief 命令表定义
 */
typedef struct shell_command {
    union {
        struct {
            int value;                // 按键键值
            int (*function)(Shell *); // 按键执行
        } key;
    };
} shell_command_list;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
int  shell_init(void);
void shell_loop(void);

void set_shell_output(void *output);
void set_shell_input(void *input);
/*---------- end of file ----------*/
#endif
