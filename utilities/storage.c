/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : storage.c
 * @Author       : lxf
 * @Date         : 2025-05-10 15:35:31
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-13 10:40:59
 * @Brief        : 简易非易失存储管理组件(不支持均衡擦写)
 * @features     :
 *               - 基于section的分区管理(链表存储)
 *               - CRC8校验 + Magic Code防错
 *               - 支持延时保存机制(防止连续写入过快)
 *               - 自动重试机制(写入失败重试3次)
 *               - 粒度对齐支持(适配Flash擦除粒度)
 *
 * @usage        :
 *               @code
 *               // 1. 创建存储句柄
 *               storage_hanle_t *storage = storage_handle_create("eeprom",
 *                           &ops, grain, base_addr, total_size);
 *
 *               // 2. 添加section
 *               uint8_t data_buf[32];
 *               int fd = storage_add_seciton_to_table(storage, data_buf, sizeof(data_buf));
 *
 *               // 3. 保存数据
 *               storage_data_save(storage, fd, 0);  // 立即保存
 *               storage_data_save(storage, fd, 100); // 延时100ms保存
 *
 *               // 4. 读取数据
 *               storage_data_read(storage, fd);
 *
 *               // 5. 轮询处理(在主循环中调用)
 *               storage_poll_ms(storage);
 *               @endcode
 *
 * @note         依赖外部实现 dev_read/dev_write 接口
 *               Section使用链表管理,数量较大时建议改用哈希表
 *
 * @warning      不支持均衡擦写,频繁写入同一区域会影响Flash寿命
 */

/*---------- includes ----------*/
#include "options.h"
#include "storage.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief 计算CRC8校验码(CRC-8-CCITT多项式0x31)
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return CRC8校验值
 */
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
 * @brief 创建存储句柄
 * @param name 句柄名称
 * @param ops 底层读写操作函数表
 * @param grain 擦除粒度(大部分NVS介质无法按字节擦除)
 * @param base_address 存储介质基地址
 * @param total_size 存储介质总大小
 * @return 存储句柄指针,失败返回NULL
 */
storage_hanle_t *
storage_handle_create(char *name, storage_ops_t *ops, uint8_t grain, uint32_t base_address, uint32_t total_size)
{
    storage_hanle_t *handle = malloc(sizeof(storage_hanle_t));

    do {
        if (handle == NULL) {
            xlog_error("malloc faild.");
            break;
        }

        if (ops == NULL) {
            xlog_error("ops is null.");
            break;
        }

        if (name == NULL) {
            xlog_error("name is null.");
            break;
        }

        /* 1. 初始化句柄参数 */
        handle->name = name;
        INIT_LIST_HEAD(&handle->section);
        handle->section_number = 0;
        handle->ops = ops;
        handle->config.grain = grain;
        handle->config.base_address = base_address;
        handle->config.total_size = total_size;
        handle->lock = false;

        /* 2. grain为0时默认设为1(避免除零错误) */
        if (handle->config.grain == 0) {
            handle->config.grain = 1;
        }

        xlog_count("storage(%s) create success. base address(%d). total size(%d)",
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
 * @return 找到的节点指针,NULL表示未找到
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
 * @brief 保存数据到存储介质
 * @note  ms=0时立即保存(阻塞,失败重试3次); ms>0时延时保存(非阻塞,由poll轮询触发)
 * @param handle 存储句柄
 * @param fd 文件描述符
 * @param ms 延时毫秒数(0=立即保存)
 * @return true=成功, false=失败
 */
bool storage_data_save(storage_hanle_t *handle, int fd, uint32_t ms)
{
    bool result = false;
    struct storage_section_node *node = NULL;
    int32_t j = 0;

    do {
        if (handle == NULL || handle->ops == NULL || handle->ops->device_read == NULL
            || handle->ops->device_write == NULL) {
            xlog_error("handle no init");
            break;
        }

        if (fd <= 0) {
            xlog_error("fd error.");
            break;
        }

        node = find_section_by_fd(handle, fd);

        if (node == NULL) {
            xlog_error("section(%d) not found.", fd);
            break;
        }

        if (ms > 0) {
            /* 延时保存模式:设置标志和倒计时,由poll轮询触发实际写入 */
            node->flag = true;
            if (node->safe_time_counts < __ms2ticks(ms)) {
                node->safe_time_counts = __ms2ticks(ms);
            }
            break;
        };

        /* 立即保存模式 */
        storage_save_t *save = malloc(node->size + 3);
        if (save == NULL) {
            xlog_error("malloc failed");
            break;
        }
        save->magic_code = STORAGE_MAIGC_CODE;
        save->crc = crc8_ccitt(node->data, node->size);
        memcpy(save->data, node->data, node->size);
        /* 写入失败重试3次,间隔5ms */
        while (j < 3) {
            if (handle->ops->device_write((uint8_t *)save, node->address, node->size + sizeof(storage_save_t))) {
                result = true;
                break;
            }
            delay_ms(5);
            j++;
        }
        if (save) {
            free(save);
            save = NULL;
        }
        if (result == false) {
            xlog_error("storage save fd(%d) error.", fd);
        }
    } while (0);

    return result;
}

/**
 * @brief 从存储介质读取数据
 * @note  读取后会校验CRC和Magic Code
 * @param handle 存储句柄
 * @param fd 文件描述符
 * @return true=成功, false=失败
 */
bool storage_data_read(storage_hanle_t *handle, int fd)
{
    bool result = false;
    struct storage_section_node *node = NULL;
    do {
        if (handle == NULL || handle->ops == NULL || handle->ops->device_read == NULL
            || handle->ops->device_write == NULL) {
            xlog_error("handle no init");
            break;
        }

        if (fd <= 0) {
            xlog_error("fd error.");
            break;
        }

        node = find_section_by_fd(handle, fd);

        if (node == NULL) {
            xlog_error("section(%d) not found.", fd);
            break;
        }

        storage_save_t *save = malloc(node->size + sizeof(storage_save_t));
        if (save == NULL) {
            xlog_error("malloc failed");
            break;
        }
        do {
            /* 1. 读取数据 */
            if (handle->ops->device_read((uint8_t *)save, node->address, node->size + sizeof(storage_save_t))
                == false) {
                xlog_error("read error.");
                break;
            }

            /* 2. 校验CRC */
            if (crc8_ccitt((const char *)save->data, node->size) != save->crc) {
                xlog_error("crc error");
                break;
            }

            /* 3. 校验Magic Code */
            if (save->magic_code != STORAGE_MAIGC_CODE) {
                xlog_error("magic code error.");
                break;
            }
            /* 4. 校验通过,拷贝数据 */
            memcpy(node->data, save->data, node->size);
            result = true;
        } while (0);

        if (save) {
            free(save);
        }
    } while (0);

    return result;
}

/**
 * @brief 存储轮询处理(需在主循环中定时调用)
 * @note  处理延时保存请求,当倒计时结束时触发实际写入操作
 * @param handle 存储句柄
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
                /* 倒计时 */
                pos->safe_time_counts--;
                if (pos->safe_time_counts != 0) {
                    continue;
                }
                /* 倒计时结束,触发保存 */
                pos->flag = false;
                storage_save_t *save = malloc(pos->size + sizeof(storage_save_t));
                if (save == NULL) {
                    continue;
                }
                int32_t j = 0;
                bool result = false;
                save->magic_code = STORAGE_MAIGC_CODE;
                save->crc = crc8_ccitt(pos->data, pos->size);
                memcpy(save->data, pos->data, pos->size);
                /* 写入失败重试3次 */
                while (j < 3) {
                    if (handle->ops->device_write((uint8_t *)save, pos->address, pos->size + sizeof(storage_save_t))) {
                        result = true;
                        break;
                    }
                    delay_ms(5);
                    j++;
                }
                if (save) {
                    free(save);
                }
                if (result == false) {
                    xlog_error("storage save fd(%d) error.", pos->fd);
                }
            }
        }
    } while (0);
}

/**
 * @brief 获取对齐后的大小
 * @note  向上对齐到grain的整数倍
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
 * @brief 添加section到存储表
 * @note  地址会根据grain自动对齐,section_offset会递增
 * @param handle 存储句柄
 * @param buf 数据缓冲区指针
 * @param size 数据大小
 * @return >=0成功返回fd, <0失败
 */
int storage_add_seciton_to_table(storage_hanle_t *handle, uint8_t *buf, uint32_t size)
{
    int fd = -1;
    uint32_t aligned_size = 0;
    do {
        struct storage_section_node *node = malloc(sizeof(struct storage_section_node));
        if (!node) {
            break;
        }
        /* 计算对齐后的偏移地址(含magic+crc共3字节) */
        aligned_size = get_aligned_size(handle->section_offset + size + sizeof(storage_save_t), handle->config.grain);

        /* 检查是否超出总大小 */
        if (aligned_size > handle->config.total_size) {
            xlog_error("storage(%s) total size(%d) is not enough. current size is %d",
                       handle->name,
                       handle->config.total_size,
                       aligned_size);
            break;
        }
        /* 初始化section节点 */
        node->fd = ++handle->section_number;
        INIT_LIST_HEAD(&node->node);
        node->data = buf;
        node->address = handle->config.base_address + handle->section_offset;
        node->size = size;
        node->flag = false;
        node->safe_time_counts = 0;

        /* 更新偏移量并添加到链表 */
        handle->section_offset = aligned_size;
        fd = node->fd;
        list_add_tail(&node->node,&handle->section);
        xlog_count("add section(%d) success. address(%d) size(%d)", fd, node->address, node->size);
    } while (0);
    return fd;
}

/*---------- end of file ----------*/
