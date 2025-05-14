/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : storage.h
 * @Author       : lxf
 * @Date         : 2025-05-10 15:35:37
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-13 08:32:17
 * @Brief        : 
 */

#ifndef __STORAGE_H__
#define __STORAGE_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "stdint.h"
#include "stdbool.h"
#include "device.h"
#include "drv_err.h"
/*---------- macro ----------*/
#define STORAGE_MAIGC_CODE 0xAABB
/*---------- type define ----------*/
#pragma pack(1)
// 报错到
typedef struct {
    uint16_t magic_code;
    uint8_t crc;
    uint8_t data[];
} storage_save_t;
#pragma pack()

typedef struct {
    void *data;
    uint16_t address;
    uint16_t size;
    uint32_t delay;
    uint8_t crc;
} storage_data_fifo_t;

typedef struct {
    storage_data_fifo_t *table;
    uint8_t table_size;
    device_t *dev;//读写操作接口
    uint8_t save_delay_ms;//某些设备不能快速保存，需要保存之间加延时。
    void (*delay_ms)(uint32_t ms);
} storage_ops_t;

typedef struct {
    char *name;
    storage_ops_t *ops;
} storage_hanle_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
storage_hanle_t *storage_handle_create(char *name, storage_ops_t *ops, device_t *dev);
void storage_handle_destroy(storage_hanle_t *handle);
bool storage_data_save(storage_hanle_t *handle,uint8_t index, uint32_t ms);
bool storage_data_read(storage_hanle_t *handle, uint8_t index);
void storage_poll_ms(storage_hanle_t *handle);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__STORAGE_H__

