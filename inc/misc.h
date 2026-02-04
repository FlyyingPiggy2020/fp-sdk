/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : misc.h
 * @Author       : lxf
 * @Date         : 2025-08-20 20:21:46
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-12 11:29:48
 * @Brief        : 通用工具宏和辅助函数定义
 */
#ifndef __MISC_H__
#define __MISC_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
/*---------- macro ----------*/
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))
#elif defined(__GNUC__)
#define container_of(ptr, type, member)                      \
    ({                                                       \
        const __typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member));   \
    })
#elif defined(_MSC_VER)
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))
#else
#error "No container_of defined in this compiler"
#endif

/**
 * @brief 格式化字符串
 * @return {*}
 */
#define _STRING(x)         #x         /*<< only format alphabet as string */
#define STRING(x)          _STRING(x) /*<< format alphabet or digit as string */

/**
 * @brief 二维数组行数
 * @return {*}
 */
#define ARRAY_SIZE(x)      (sizeof(x) / sizeof((x)[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t *)0)->f))

/**
 * @brief 颜色定义
 * @return {*}
 */
#define COLOR_RED          "\033[31;22m"
#define COLOR_GREEN        "\033[32;22m"
#define COLOR_YELLOW       "\033[33;22m"
#define COLOR_BLUE         "\033[34;22m"
#define COLOR_PURPLE       "\033[35;22m"
#define COLOR_SKY_BLUE     "\033[36;22m"
#define COLOR_WHITE        "\033[37;22m"

/**
 * @brief 小端浮点数字节转换联合体
 * @note 用于小端格式的浮点数/整数数据转换
 */
typedef union {
    float f;
    int32_t i;
    uint32_t u;
    uint8_t b[4];
} le_float_bytes_t;

/**
 * @brief 从小端字节序读取浮点数
 * @param  ptr: 字节数组指针
 * @return float 转换后的浮点数
 */
#define read_float_le(ptr)                                             \
    ({                                                                 \
        float val;                                                     \
        uint8_t bytes[4] = { (ptr)[0], (ptr)[1], (ptr)[2], (ptr)[3] }; \
        memcpy(&val, bytes, 4);                                        \
        val;                                                           \
    })

/**
 * @brief 向小端字节序写入浮点数
 * @param  ptr: 字节数组指针
 * @param  val: 要写入的浮点数值
 */
#define write_float_le(ptr, val)  \
    do {                          \
        uint8_t bytes[4];         \
        memcpy(bytes, &(val), 4); \
        (ptr)[0] = bytes[0];      \
        (ptr)[1] = bytes[1];      \
        (ptr)[2] = bytes[2];      \
        (ptr)[3] = bytes[3];      \
    } while (0)

/**
 * @brief 从小端字节序读取16位整数
 * @param  ptr: 字节数组指针
 * @return uint16_t 转换后的16位整数
 */
#define read_u16_le(ptr) ((uint16_t)((ptr)[0] | ((ptr)[1] << 8)))

/**
 * @brief 从小端字节序读取32位整数
 * @param  ptr: 字节数组指针
 * @return uint32_t 转换后的32位整数
 */
#define read_u32_le(ptr) ((uint32_t)((ptr)[0] | ((ptr)[1] << 8) | ((ptr)[2] << 16) | ((ptr)[3] << 24)))

/**
 * @brief 向小端字节序写入16位整数
 * @param  ptr: 字节数组指针
 * @param  val: 要写入的16位整数值
 */
#define write_u16_le(ptr, val)          \
    do {                                \
        (ptr)[0] = (uint8_t)(val);      \
        (ptr)[1] = (uint8_t)(val >> 8); \
    } while (0)

/**
 * @brief 向小端字节序写入32位整数
 * @param  ptr: 字节数组指针
 * @param  val: 要写入的32位整数值
 */
#define write_u32_le(ptr, val)           \
    do {                                 \
        (ptr)[0] = (uint8_t)(val);       \
        (ptr)[1] = (uint8_t)(val >> 8);  \
        (ptr)[2] = (uint8_t)(val >> 16); \
        (ptr)[3] = (uint8_t)(val >> 24); \
    } while (0)

/*---------- type define ----------*/
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
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
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
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__MISC_H__
