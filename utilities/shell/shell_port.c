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
 * @FilePath     : shell_port.c
 * @Author       : lxf
 * @Date         : 2024-02-23 14:49:00
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-23 14:49:01
 * @Brief        : shell port
 */

/*---------- includes ----------*/

#include "fp_sdk.h"
#include "main.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/

void (*shell_output)(const char *buffer, fp_size_t size) = NULL;
fp_size_t (*shell_input)(char *log, fp_size_t wsize) = NULL;
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

void set_shell_output(void *output)
{
    assert(output);
    shell_output = output;
}

void set_shell_input(void *input)
{
    assert(input);
    shell_input = input;
}
/*---------- end of file ----------*/
