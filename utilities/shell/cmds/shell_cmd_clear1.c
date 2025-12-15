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
 * @FilePath     : shell_cmd_clear1.c
 * @Author       : lxf
 * @Date         : 2024-03-05 17:21:04
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-05 17:22:31
 * @Brief        :
 */

/*---------- includes ----------*/

#include "../inc/shell.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

extern void shell_output(Shell *shell, const char *format, ...);
extern void shell_prompt(Shell *shell, uint8_t is_new_line);
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief clear命令，测试tab补全
 * @return {*}
 */
int clear1(Shell *shell, uint8_t argc, char *argv[])
{
    if (argc == 1) {
        shell_output(shell, "\033[2J\033[3J\033[1H");
        shell_prompt(shell, 0);
    }
    return 0;
}
SHELL_EXPORT_CMD(clear12, clear1);
/*---------- end of file ----------*/
