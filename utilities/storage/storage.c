/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : storage.c
 * @Author       : lxf
 * @Date         : 2025-05-10 15:35:31
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-12 13:30:34
 * @Brief        : 非常简单的数据管理组件(不支持均衡擦写)依赖于export组件
 * 1.实现一个storage_data_fifo_t的表格(此表格每个项需要比待保存的数据多3个字节，用于保存crc和magic code)
 * 2.利用device框架，自行实现自己的底层读写接口。
 *
 */

/*---------- includes ----------*/
#include "heap.h"
#include "storage.h"

#define LOG_TAG "storage"
#include "fplog.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static unsigned char crc8_ccitt(const char *buf, int len)
{
    char *ptr = (char *)buf;
    register int counter;
    register unsigned char crc = 0;

    while (len--) {
        crc ^= *ptr++;
        for (counter = 0; counter < 8; counter++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief 创建一个存储句柄
 * @param {storage_data_fifo_t} *table
 * @param {uint16_t} table_size
 * @return {*}
 */
storage_hanle_t *storage_handle_create(char *name, storage_ops_t *ops, device_t *dev)
{
    storage_hanle_t *handle = malloc(sizeof(storage_hanle_t));
    if (handle == NULL) {
        return NULL;
    }
    handle->name = name;
    handle->ops = ops;
    handle->ops->dev = dev;
    return handle;
}

/**
 * @brief 删除一个storage句柄
 * @param {storage_hanle_t} *handle
 * @return {*}
 */
void storage_handle_destroy(storage_hanle_t *handle)
{
    if (handle == NULL) {
        return;
    }
    free(handle);
    handle = NULL;
}

/**
 * @brief 延时ms毫秒,写入数据。(如果ms为0，当写入失败的时候会重复3次。该行为是阻塞的)
 * @param {void} index
 * @param {uint32_t} ms
 * @return {*} true 成功 false 失败
 */
bool storage_data_save(storage_hanle_t *handle, uint8_t index, uint32_t ms)
{
    int32_t j = 0;
    if (handle == NULL || handle->ops->table == NULL || handle->ops == NULL) {
        log_e("handle or table is null");
        return false;
    }

    if (index > handle->ops->table_size) {
        log_e("index error.");
        return false;
    }
    storage_data_fifo_t *ptable = &handle->ops->table[index];
    if (ptable->data != NULL) {
        if (ms > 0) {
            ptable->crc = crc8_ccitt(ptable->data, ptable->size);
            ptable->delay = ms;
            return true;
        } else {
            ptable->crc = crc8_ccitt(ptable->data, ptable->size);
            ptable->delay = ms;
            storage_save_t *save = malloc(ptable->size + 3);
            save->magic_code = STORAGE_MAIGC_CODE;
            save->crc = ptable->crc;
            memcpy(save->data, ptable->data, ptable->size);
            while (j < 3) {
                int32_t ret = device_write(handle->ops->dev, save, ptable->address, ptable->size + 3);
                if (ret == DRV_ERR_OK) {
                    free(save);
                    return true;
                }
                if (handle->ops->save_delay_ms && handle->ops->delay_ms) {
                    handle->ops->delay_ms(handle->ops->save_delay_ms); // 该延时函数由用户自行实现
                }
                j++;
            }
            free(save);
        }
    }
    log_e("save error. index = %d", index);
    return false;
}

/**
 * @brief 读取数据
 * @param {void} *data
 * @return {*} true 成功 false 失败
 */
bool storage_data_read(storage_hanle_t *handle, uint8_t index)
{
    if (handle == NULL || handle->ops->table == NULL || handle->ops == NULL) {
        return false;
    }

    if (index > handle->ops->table_size) {
        log_e("index error.");
        return false;
    }

    storage_data_fifo_t *ptable = &handle->ops->table[index];
    if (ptable->data == NULL) {
        log_e("data is null.");
        return false;
    }

    storage_save_t *save = malloc(ptable->size + 3);
    if (save == NULL) {
        log_e("malloc failed");
        return false;
    }

    int32_t ret = device_read(handle->ops->dev, save, ptable->address, ptable->size + 3);
    if (ret != DRV_ERR_OK) {
        log_e("read error.");
        goto failed;
    }

    if (crc8_ccitt((const char *)save->data, ptable->size) != save->crc) {
        log_e("crc error");
        goto failed;
    }
    if (save->magic_code != STORAGE_MAIGC_CODE) {
        log_e("magic code error.");
        goto failed;
    }
    memcpy(ptable->data, save->data, ptable->size);
    free(save);
    return true;
failed:
    free(save);
    return false;
}

/**
 * @brief storage轮询(每毫秒轮询一次)
 * @param {storage_hanle_t} *handle
 * @return {*}
 */
void storage_poll_ms(storage_hanle_t *handle)
{
    if (handle == NULL) {
        return;
    }
    for (uint32_t i = 0; i < handle->ops->table_size; i++) {
        if (handle->ops->table[i].delay) {
            handle->ops->table[i].delay--;
            if (handle->ops->table[i].delay == 0) {
                // 保存数据
                int j = 0;
                int ret = 0;
                storage_data_fifo_t *ptable = &handle->ops->table[i];
                ptable->crc = crc8_ccitt(ptable->data, ptable->size);
                storage_save_t *save = malloc(ptable->size + 3);
                save->magic_code = STORAGE_MAIGC_CODE;
                save->crc = ptable->crc;
                memcpy(save->data, ptable->data, ptable->size);
                while (j < 3) {
                    ret = device_write(handle->ops->dev, save, ptable->address, ptable->size + 3);
                    if (ret == DRV_ERR_EOK) {
                        free(save);
                        break;
                    }
                    if (handle->ops->save_delay_ms && handle->ops->delay_ms) {
                        handle->ops->delay_ms(handle->ops->save_delay_ms); // 该延时函数由用户自行实现
                    }
                    j++;
                }
                free(save);
                if (ret != DRV_ERR_OK) {
                    log_e("storage error. handle:%s, index = %d.", handle->name, i);
                }
            }
        }
    }
}
/*---------- end of file ----------*/
