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
 * @FilePath     : heap.c
 * @Author       : lxf
 * @Date         : 2024-03-06 13:25:47
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-06 13:25:51
 * @Brief        : 使用tlsf作为内存管理,这个内存管理算法有一定的空间浪费,时间复杂度是O1
 */

#if (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
__asm(".global __use_no_heap_region\n\t"); // AC6申明不使用C库的堆
#elif defined(__CC_ARM)
#pragma import(__use_no_heap_region) // AC5申明不使用C库的堆
#else
#endif
/*---------- includes ----------*/

#include "fp_sdk.h"
/*---------- macro ----------*/

#define POOL_SIZE 1024 * 10 //实际上的P0OL_SIZE有大约4KB做位图了，所以这个要比实际大一点才行。
/*---------- type define ----------*/

static char heap_pool[POOL_SIZE];
static void *shell_malloc_ptr = NULL;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

int heap_init(void)
{
    int free_mem;
    free_mem = init_memory_pool(POOL_SIZE, heap_pool);
    return 0;
}
INIT_BOARD_EXPORT(heap_init);

void *malloc(size_t size)
{
    return tlsf_malloc(size);
}

void free(void *p)
{
    tlsf_free(p);
}

void *realloc(void *p, size_t want)
{
    return tlsf_realloc(p, want);
}

void *calloc(size_t nmemb, size_t size)
{
    return tlsf_calloc(nmemb, size);
}

/**
 * @brief 获取已使用的heap size
 * @return {*}
 */
int heap_get_used_size(Shell *shell, uint8_t argc, char *argv[])
{
    size_t used_heap_size = get_used_size(heap_pool);
    log_i("Total used memory= %d\n", used_heap_size);
    return 0;
}
SHELL_EXPORT_CMD(heap_used_size, heap_get_used_size);

/**
 * @brief 获取已使用的heap size
 * @return {*}
 */
int heap_malloc(Shell *shell, uint8_t argc, char *argv[])
{
    if(argc == 1) {
        shell_malloc_ptr = malloc(1024);
    }
    return 0;
}
SHELL_EXPORT_CMD(heap_malloc, heap_malloc);

/**
 * @brief 获取已使用的heap size
 * @return {*}
 */
int heap_free(Shell *shell, uint8_t argc, char *argv[])
{
    if(argc == 1 && shell_malloc_ptr != NULL) {
        free(shell_malloc_ptr);
        shell_malloc_ptr = NULL;
    }
    return 0;
}
SHELL_EXPORT_CMD(heap_free, heap_free);
/*---------- end of file ----------*/
