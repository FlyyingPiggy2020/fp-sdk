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
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : fp_sdk.h
 * @Author       : lxf
 * @Date         : 2023-12-27 18:29:53
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-27 18:29:54
 * @Brief        : fp-sdk头文件
 */

#ifndef __FP_SDK_H__
#define __FP_SDK_H__
/*---------- includes ----------*/

/**
 * @brief 是否使用SHELL
 * @return {*}
 */
#define FP_USE_SHELL 1

// #include "utilities/clists/inc/clists.h" //这条需要放到前面因为后面的文件里用到了链表
// #include "utilities/common/inc/fp_def.h"
// #include "utilities/export/inc/export.h"
// #include "device/core/inc/device_manager.h"
// #include "utilities/pingpong/inc/pingpong.h"

// #include "utilities/heap/TLSF-2.4.6/src/tlsf.h"
// #include "utilities/log/inc/log.h"
// #if (FP_USE_SHELL == 1)
// #include "utilities/shell/inc/shell.h"
// #endif

/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

int heap_init(void);
/*---------- end of file ----------*/
#endif
