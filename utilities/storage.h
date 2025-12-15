/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : storage.h
 * @Author       : lxf
 * @Date         : 2025-05-10 15:35:37
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-10-16 10:23:18
 * @Brief        :
 *
 * 2025-10-14 14:54:05 lxf 将原先的内存表格改为用storage_add_seciton_to_table函数动态添加
 */

#ifndef __STORAGE_H__
#define __STORAGE_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "stdbool.h"
#include "device.h"
#include "drv_err.h"
#include "clists.h"
/*---------- macro ----------*/
#define STORAGE_MAIGC_CODE 0xAABB
/*---------- type define ----------*/

// TODO :gcc语法不兼容,如果移植到其他编译器,需要用更加通用的写法
#pragma pack(1)
typedef struct {
    uint16_t magic_code;
    uint8_t crc;
    uint8_t data[];
} storage_save_t;
#pragma pack()

struct storage_section_node {
    int fd;
    struct list_head node;
    void *data;       // 保存的数据的指针
    uint32_t address; // 存储介质中的起始地址
    uint32_t size;    // 大小
    bool flag;        // 是否准备存储
    uint32_t safe_time_counts;
};

typedef struct {
    bool (*device_write)(uint8_t *pbuf, uint32_t address, uint32_t size); // ture:成功 false:失败
    bool (*device_read)(uint8_t *pbuf, uint32_t address, uint32_t size);  // ture:成功 false:失败
    void (*delay_ms)(uint32_t ms);                                        // 延时函数
} storage_ops_t;

typedef struct {
    uint8_t grain;
    uint32_t base_address;
    uint32_t total_size;
} storage_config_t;

typedef struct {
    char *name;
    struct list_head section;
    uint8_t section_number;
    uint32_t section_offset;
    storage_ops_t *ops;
    storage_config_t config;
    bool lock;
} storage_hanle_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
storage_hanle_t *
storage_handle_create(char *name, storage_ops_t *ops, uint8_t grain, uint32_t base_address, uint32_t total_size);
bool storage_data_save(storage_hanle_t *handle, int fd, uint32_t ms);
bool storage_data_read(storage_hanle_t *handle, int fd);
void storage_poll_ms(storage_hanle_t *handle);
int storage_add_seciton_to_table(storage_hanle_t *handle, uint8_t *buf, uint32_t size);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__STORAGE_H__
