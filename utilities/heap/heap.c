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
 * @Brief        : 使用tlsf作为内存管理
 */

#if (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
__asm(".global __use_no_heap_region\n\t"); // AC6申明不使用C库的堆
#elif defined(__CC_ARM)
#pragma import(__use_no_heap_region) // AC5申明不使用C库的堆
#else
#endif
/*---------- includes ----------*/
#include "bsp_config.h"
#include "TLSF-2.4.6/src/tlsf.h"
#include "export.h"
#include "stdlib.h"
/*---------- macro ----------*/
#define RT_ALIGN(size, align)      (((size) + (align)-1) & ~((align)-1))
#define RT_ALIGN_DOWN(size, align) ((size) & ~((align)-1))
#define RT_ALIGN_SIZE              8

#define _SRAM_SIZE                 20
#define _SRAM_END                  (0x20000000 + _SRAM_SIZE * 1024)
#if defined(__ARMCC_VERSION)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section = "CSTACK"
#define HEAP_BEGIN (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define HEAP_BEGIN ((void *)&__bss_end)
#endif

#define HEAP_END _SRAM_END

#if (CONFIG_TLSF_HEAP_SIZE == 0)
#define POOL_SIZE (HEAP_END - (uint32_t)HEAP_BEGIN)
#else
#define POOL_SIZE CONFIG_TLSF_HEAP_SIZE
static char heap_pool[POOL_SIZE] __attribute__((aligned(4)));
#endif

/*---------- type define ----------*/

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static void rt_system_heap_init(void *begin_addr, void *end_addr)
{
    uint32_t begin_align = RT_ALIGN((uint32_t)begin_addr, RT_ALIGN_SIZE);
    uint32_t end_align = RT_ALIGN_DOWN((uint32_t)end_addr, RT_ALIGN_SIZE);
    /* Initialize system memory heap */
    init_memory_pool(end_align - begin_align, begin_addr);
}

int heap_init(void)
{
#if (CONFIG_TLSF_HEAP_SIZE == 0)
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#else
    init_memory_pool(POOL_SIZE, heap_pool);
#endif
}
INIT_BOARD_EXPORT(heap_init);

#if defined(__ARMCC_VERSION)
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
#endif
#if (FP_USE_SHELL == 1)
/**
 * @brief 获取已使用的heap size
 * @return {*}
 */
int heap_get_used_size(Shell *shell, uint8_t argc, char *argv[])
{
    size_t used_heap_size = get_used_size(heap_pool);
    shell->shell_write("\r\n", 2);
    log_i("Used memory= %d.Total memory size = %d.", used_heap_size, POOL_SIZE);
    return 0;
}
SHELL_EXPORT_CMD(heap_used_size, heap_get_used_size);
#else
int heap_get_used_size(void)
{
#if (CONFIG_TLSF_HEAP_SIZE == 0)
    return get_used_size(HEAP_BEGIN);
#else
    return get_used_size(heap_pool);
#endif
}
#endif
/*---------- end of file ----------*/
