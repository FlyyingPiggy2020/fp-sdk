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
/*---------- macro ----------*/

/* https://www.asciitable.com/ */

#define SHELL_TAB 0X09
#define SHELL_UP  "^[[A"
#define SHELL_DO  "^[[B"
#define SHELL_RT  "^[[C"
#define SHELL_LT  "^[[D"

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

void set_shell_output(void *output);
void set_shell_input(void *input);
/*---------- end of file ----------*/
#endif
