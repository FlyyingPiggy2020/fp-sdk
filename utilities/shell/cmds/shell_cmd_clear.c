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
 * @FilePath     : shell_cmd_clear.c
 * @Author       : lxf
 * @Date         : 2024-03-01 11:53:35
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-02 08:42:45
 * @Brief        : shell内置命令clear
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

#if (USE_ESP == 1)
void esp32_cmd_clear_link_hook(void)
{
}
#endif
/**
 * @brief clear命令
 * @return {*}
 */
int clear(Shell *shell, uint8_t argc, char *argv[])
{
    if (argc == 1) {
        shell_output(shell, "\033[2J\033[3J\033[1H");
    }
    return 0;
}
SHELL_EXPORT_CMD(clear, clear);
/*---------- end of file ----------*/
