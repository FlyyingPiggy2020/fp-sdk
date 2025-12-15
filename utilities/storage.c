/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : storage.c
 * @Author       : lxf
 * @Date         : 2025-05-10 15:35:31
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-10-16 10:32:49
 * @Brief        : 非常简单的数据管理组件(不支持均衡擦写)依赖于export组件
 * 1.实现一个storage_data_fifo_t的表格(此表格每个项需要比待保存的数据多3个字节，用于保存crc和magic code)
 * 2.利用device框架，自行实现自己的底层读写接口。依赖于dev_write和dev_read接口。
 * 3.用链表保存section，在嵌入式系统中使用，遍历的效率不高。如果数量级很大，需要改用哈希表。
 *
 * 2025-10-14 14:54:05 lxf 重构:将静态的内存表改为使用storage_add_seciton_to_table函数动态添加。
 *
 */

/*---------- includes ----------*/
#define LOG_TAG "STORAGE"
#include "heap.h"
#include <stdbool.h>
#include "storage.h"

#define LOG_TAG "storage"
#include "options.h"

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
 * @param {char} *name 句柄的名称
 * @param {storage_ops_t} *ops 句柄的操作函数
 * @param {uint8_t} grain 大部分nvs介质都有粒度，无法按字节擦除。有一个擦除的最小颗粒度。
 * @param {uint16_t} safe_time 某些nvs介质连续存储过快会出问题，这个参数可以限制两次存储之间的最短时间。
 * @param {uint16_t} base_address 该存储介质的基地址
 * @param {uint16_t} total_size 该存储介质的最大大小
 * @return {*}
 */
storage_hanle_t *
storage_handle_create(char *name, storage_ops_t *ops, uint8_t grain, uint32_t base_address, uint32_t total_size)
{
    storage_hanle_t *handle = __malloc(sizeof(storage_hanle_t));

    do {
        if (handle == NULL) {
            log_e("malloc faild.");
            break;
        }

        if (ops == NULL) {
            log_e("ops is null.");
            break;
        }

        if (name == NULL) {
            log_e("name is null.");
            break;
        }

        handle->name = name;
        INIT_LIST_HEAD(&handle->section);
        handle->section_number = 0;
        handle->ops = ops;
        handle->config.grain = grain;
        handle->config.base_address = base_address;
        handle->config.total_size = total_size;
        handle->lock = false;

        /* confict */
        if (handle->config.grain == 0) {
            handle->config.grain = 1;
        }

        log_i("storage(%s) create success. base address(%d). total size(%d)",
              handle->name,
              handle->config.base_address,
              handle->config.total_size);
    } while (0);

    return handle;
}

/**
 * @brief 删除一个storage句柄
 * @param {storage_hanle_t} *handle
 * @return {*}
 */
// void storage_handle_destroy(storage_hanle_t *handle)
//{
//     if (handle == NULL) {
//         return;
//     }
//     // TODO : 未实现，一般嵌入式中不需要动态删除句柄。只创建不删除
// }

/**
 * @brief 通过fd查找section节点
 * @param handle 存储句柄
 * @param fd 文件描述符
 * @return 找到的节点指针，NULL表示未找到
 */
static struct storage_section_node *find_section_by_fd(storage_hanle_t *handle, uint8_t fd)
{
    struct storage_section_node *pos;

    list_for_each_entry(pos, struct storage_section_node, &handle->section, node)
    {
        if (pos->fd == fd) {
            return pos;
        }
    }

    return NULL;
}
/**
 * @brief 延时ms毫秒,写入数据。(如果ms为0，当写入失败的时候会重复3次。该行为是阻塞的)
 * @param {void} index
 * @param {uint32_t} ms ms=0时立即保存, ms>0时，延时ms毫秒之后保存数据
 * @return {*} true 成功 false 失败
 */
bool storage_data_save(storage_hanle_t *handle, int fd, uint32_t ms)
{
    bool result = false;
    struct storage_section_node *node = NULL;
    int32_t j = 0;

    do {
        if (handle == NULL || handle->ops == NULL || handle->ops->delay_ms == NULL || handle->ops->device_read == NULL
            || handle->ops->device_write == NULL) {
            log_e("handle no init");
            break;
        }

        if (fd < 0) {
            log_e("fd error.");
            break;
        }

        node = find_section_by_fd(handle, fd);

        if (node == NULL) {
            log_e("section(%d) not found.", fd);
            break;
        }

        if (ms > 0) {
            node->flag = true;
            if (node->safe_time_counts < __ms2ticks(ms)) {
                node->safe_time_counts = __ms2ticks(ms);
            }
            break;
        };

        storage_save_t *save = __malloc(node->size + 3);
        if (save == NULL) {
            log_e("malloc failed");
            break;
        }
        save->magic_code = STORAGE_MAIGC_CODE;
        save->crc = crc8_ccitt(node->data, node->size);
        memcpy(save->data, node->data, node->size);
        while (j < 3) {
            if (handle->ops->device_write((uint8_t *)save, node->address, node->size + sizeof(storage_save_t))) {
                result = true;
                break;
            }
            handle->ops->delay_ms(5);
            j++;
        }
        if (save) {
            __free(save);
            save = NULL;
        }
        if (result == false) {
            log_e("storage save fd(%d) error.", fd);
        }
    } while (0);

    return result;
}

/**
 * @brief 读取数据
 * @param {void} *data
 * @return {*} true 成功 false 失败
 */
bool storage_data_read(storage_hanle_t *handle, int fd)
{
    bool result = false;
    struct storage_section_node *node = NULL;
    do {
        if (handle == NULL || handle->ops == NULL || handle->ops->delay_ms == NULL || handle->ops->device_read == NULL
            || handle->ops->device_write == NULL) {
            log_e("handle no init");
            break;
        }

        if (fd < 0) {
            log_e("fd error.");
            break;
        }

        node = find_section_by_fd(handle, fd);

        if (node == NULL) {
            log_e("section(%d) not found.", fd);
            break;
        }

        storage_save_t *save = __malloc(node->size + 3);
        if (save == NULL) {
            log_e("malloc failed");
            break;
        }
        do {
            if (handle->ops->device_read((uint8_t *)save, node->address, node->size + sizeof(storage_save_t))
                == false) {
                log_e("read error.");
                break;
            }

            if (crc8_ccitt((const char *)save->data, node->size) != save->crc) {
                log_e("crc error");
                break;
            }

            if (save->magic_code != STORAGE_MAIGC_CODE) {
                log_e("magic code error.");
                break;
            }
            memcpy(node->data, save->data, node->size);
            result = true;
        } while (0);

        if (save) {
            __free(save);
        }
    } while (0);

    return result;
}

/**
 * @brief storage轮询
 * @param {storage_hanle_t} *handle
 * @return {*}
 */
void storage_poll_ms(storage_hanle_t *handle)
{
    struct storage_section_node *pos;
    do {
        if (handle == NULL) {
            break;
        }

        if (handle->lock) {
            break;
        }

        list_for_each_entry(pos, struct storage_section_node, &handle->section, node)
        {
            if (pos->flag == false || pos->data == NULL) {
                continue;
            }
            if (pos->safe_time_counts) {
                pos->safe_time_counts--;
                if (pos->safe_time_counts != 0) {
                    continue;
                }
                pos->flag = false;
                storage_save_t *save = __malloc(pos->size + sizeof(storage_save_t));
                if (save == NULL) {
                    continue;
                }
                int32_t j = 0;
                bool result = false;
                save->magic_code = STORAGE_MAIGC_CODE;
                save->crc = crc8_ccitt(pos->data, pos->size);
                memcpy(save->data, pos->data, pos->size);
                while (j < 3) {
                    if (handle->ops->device_write((uint8_t *)save, pos->address, pos->size + sizeof(storage_save_t))) {
                        result = true;
                        break;
                    }
                    handle->ops->delay_ms(5);
                    j++;
                }
                if (save) {
                    __free(save);
                }
                if (result == false) {
                    log_e("storage save fd(%d) error.", pos->fd);
                }
            }
        }
    } while (0);
}

/**
 * @brief 获取对齐后的大小
 * @param size 原始大小
 * @param grain 对齐粒度
 * @return 对齐后的大小
 */
static uint32_t get_aligned_size(uint32_t size, uint16_t grain)
{
    // todo: 这个函数并不完善。如果size接近uint32_t的最大值的时候，size + grain会溢出。
    // 目前应用场景下这个取值范围够用
    if (grain == 0) {
        return size;
    }
    return ((size + grain - 1) / grain) * grain;
}

/**
 * @brief 创建一个section
 * @param {storage_hanle_t} *handle
 * @return {*} fd > 0 返回操作句柄 , fd < 0 section添加失败
 */
int storage_add_seciton_to_table(storage_hanle_t *handle, uint8_t *buf, uint32_t size)
{
    int fd = -1;
    uint32_t aligned_size = 0;
    do {
        struct storage_section_node *node = __malloc(sizeof(struct storage_section_node));
        if (!node) {
            break;
        }
        // aligned_size会根据grain颗粒度对齐
        aligned_size = get_aligned_size(handle->section_offset + size + sizeof(storage_save_t), handle->config.grain);

        if (aligned_size > handle->config.total_size) {
            log_e("storage(%s) total size(%d) is not enough. current size is %d",
                  handle->name,
                  handle->config.total_size,
                  aligned_size);
            break;
        }
        node->fd = handle->section_number++;
        INIT_LIST_HEAD(&node->node);
        node->data = buf;
        node->address = handle->config.base_address + handle->section_offset;
        node->size = size;
        node->flag = false;
        node->safe_time_counts = 0;

        handle->section_offset = aligned_size;
        fd = node->fd;
        list_add_tail(&handle->section, &node->node);
        log_i("add section(%d) success. address(%d) size(%d)", fd, node->address, node->size);
    } while (0);
    return fd;
}

/*---------- end of file ----------*/
