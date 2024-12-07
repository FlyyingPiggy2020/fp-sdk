/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : drvier.h
 * @Author       : lxf
 * @Date         : 2024-12-05 10:49:38
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-05 13:21:29
 * @Brief        : 从岑老板的代码中节选了一部分
 */

#ifndef __DRIVER_H__
#define __DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include "bsp_config.h" //该头文件来自于board组件
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/*---------- macro ----------*/
#undef _DEV_SECTION_PREFIX
#if defined(__linux__) || defined(_WIN32)
#define _DEV_SECTION_PREFIX
#else
#define _DEV_SECTION_PREFIX "."
#endif

#define DRIVER_DEFINED(name, open, close, write, read, ioctl, irq_handler)                   \
    driver_t driver_##name __attribute__((used, section(_DEV_SECTION_PREFIX "drv_defined"))) \
    __attribute__((aligned(4))) = { #name, open, close, write, read, ioctl, irq_handler }

#define ARRAY_SIZE(x)      (sizeof(x) / sizeof((x)[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t *)0)->f))
/*---------- type define ----------*/
typedef struct st_driver {
    char drv_name[10];
    int32_t (*open)(struct st_driver **drv);
    void (*close)(struct st_driver **drv);
    int32_t (*write)(struct st_driver **drv, void *buf, uint32_t addition, uint32_t len);
    int32_t (*read)(struct st_driver **drv, void *buf, uint32_t addition, uint32_t len);
    int32_t (*ioctl)(struct st_driver **drv, uint32_t cmd, void *args);
    int32_t (*irq_handler)(struct st_driver **drv, uint32_t irq_handler, void *args, uint32_t len);
} driver_t;

typedef struct protocol_callback *protocol_callback_t;
struct protocol_callback {
    uint32_t type;
    void *cb;
};

typedef struct protocol_callback_strings *protocol_callback_strings_t;
struct protocol_callback_strings {
    const char *name;
    void *cb;
};

static inline void *protocol_callback_find(uint32_t type, void *tables, uint32_t table_size)
{
    protocol_callback_t cb_tables = (protocol_callback_t)tables;
    void *cb = NULL;

    for (uint16_t i = 0; i < table_size; ++i) {
        if (cb_tables[i].type == type) {
            cb = cb_tables[i].cb;
            break;
        }
    }

    return cb;
}

static inline void *protocol_callback_strings_find(const char *name, void *tables, uint32_t table_size)
{
    protocol_callback_strings_t cb_tables = (protocol_callback_strings_t)tables;
    void *cb = NULL;

    for (uint16_t i = 0; i < table_size; ++i) {
        if (strcmp(name, cb_tables[i].name) == 0 && strlen(name) == strlen(cb_tables[i].name)) {
            cb = cb_tables[i].cb;
            break;
        }
    }

    return cb;
}
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
int32_t driver_search_device(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
