/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : pixel_framebuffer.h
 * @Author       : lxf
 * @Date         : 2026-03-09 18:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-09 21:00:00
 * @Brief        : 通用像素帧缓冲工具
 * @features     :
 *               - 提供 front/back 双缓冲，避免显示撕裂
 *               - 支持区域写入、单像素写入和范围填充
 *               - 提供 commit/poll 机制，适配异步输出设备
 *               - 后端通过 output_ops 抽象，可复用到不同像素类设备
 * @note         :
 *               - 本模块只关注“帧数据如何组织和提交”，不关心具体输出硬件
 *               - 上层可以是主机协议栈，也可以是 MCU 本地灯效逻辑
 */

#ifndef __PIXEL_FRAMEBUFFER_H__
#define __PIXEL_FRAMEBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/**
 * @brief  Pixel color in RGB order
 * @note   该结构体用于像素级操作接口，字节顺序与对外帧格式保持一致。
 */
struct pixel_framebuffer_color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

/**
 * @brief  Framebuffer basic information
 */
struct pixel_framebuffer_info {
    uint16_t pixel_num;      /* 像素总数。 */
    uint8_t bytes_per_pixel; /* 每个像素占用的字节数。 */
    uint32_t frame_size;     /* 整帧字节数。 */
};

/**
 * @brief  Output backend operation table
 * @note   submit() 负责提交 front buffer；is_busy() 用于查询后端是否仍在发送。
 */
struct pixel_framebuffer_output_ops {
    int32_t (*submit)(void *ctx, const uint8_t *frame, uint32_t len);
    bool (*is_busy)(void *ctx);
};

/**
 * @brief  Pixel framebuffer object
 * @note   front_buf 保存最近一次已经提交的帧，back_buf 供上层持续修改。
 *         commit() 时交换前后缓冲；若后端忙，则通过 pending_commit 延后发送。
 */
struct pixel_framebuffer {
    uint8_t *front_buf;
    uint8_t *back_buf;
    uint16_t pixel_num;
    uint8_t bytes_per_pixel;
    uint32_t frame_size;
    bool dirty;
    bool pending_commit;
    const struct pixel_framebuffer_output_ops *ops;
    void *ops_ctx;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  Initialize framebuffer object
 * @param  fb: 帧缓冲对象
 * @param  pixel_num: 像素数量
 * @param  bytes_per_pixel: 每像素字节数
 * @param  ops: 输出后端操作表
 * @param  ops_ctx: 输出后端上下文
 * @return E_OK=成功, 其他=失败
 */
int32_t pixel_framebuffer_init(struct pixel_framebuffer *fb,
                               uint16_t pixel_num,
                               uint8_t bytes_per_pixel,
                               const struct pixel_framebuffer_output_ops *ops,
                               void *ops_ctx);

/**
 * @brief  Deinitialize framebuffer object
 * @param  fb: 帧缓冲对象
 */
void pixel_framebuffer_deinit(struct pixel_framebuffer *fb);

/**
 * @brief  Query framebuffer basic information
 * @param  fb: 帧缓冲对象
 * @param  info: 输出信息结构体
 * @return E_OK=成功, 其他=失败
 */
int32_t pixel_framebuffer_get_info(struct pixel_framebuffer *fb, struct pixel_framebuffer_info *info);

/**
 * @brief  Get writable back buffer pointer
 * @param  fb: 帧缓冲对象
 * @param  ppbuf: 返回 back buffer 指针
 * @return E_OK=成功, 其他=失败
 * @note   直接写 back buffer 后应调用 mark_dirty() 或 commit()。
 */
int32_t pixel_framebuffer_get_back_buffer(struct pixel_framebuffer *fb, uint8_t **ppbuf);

/**
 * @brief  Mark back buffer as modified
 * @param  fb: 帧缓冲对象
 * @return E_OK=成功, 其他=失败
 */
int32_t pixel_framebuffer_mark_dirty(struct pixel_framebuffer *fb);

/**
 * @brief  Write one region into back buffer
 * @param  fb: 帧缓冲对象
 * @param  offset: 起始偏移
 * @param  data: 输入数据
 * @param  len: 数据长度
 * @return E_OK=成功, 其他=失败
 */
int32_t
pixel_framebuffer_write_region(struct pixel_framebuffer *fb, uint32_t offset, const uint8_t *data, uint32_t len);

/**
 * @brief  Set one pixel in back buffer
 * @param  fb: 帧缓冲对象
 * @param  index: 像素索引
 * @param  color: RGB 颜色值
 * @return E_OK=成功, 其他=失败
 */
int32_t pixel_framebuffer_set_pixel(struct pixel_framebuffer *fb, uint16_t index, struct pixel_framebuffer_color color);

/**
 * @brief  Fill one pixel range in back buffer
 * @param  fb: 帧缓冲对象
 * @param  start: 起始像素索引
 * @param  count: 填充数量，传 UINT16_MAX 表示从 start 填到末尾
 * @param  color: RGB 颜色值
 * @return E_OK=成功, 其他=失败
 */
int32_t pixel_framebuffer_fill(struct pixel_framebuffer *fb,
                               uint16_t start,
                               uint16_t count,
                               struct pixel_framebuffer_color color);

/**
 * @brief  Clear back buffer to zero
 * @param  fb: 帧缓冲对象
 * @return E_OK=成功, 其他=失败
 */
int32_t pixel_framebuffer_clear(struct pixel_framebuffer *fb);

/**
 * @brief  Commit current back buffer
 * @param  fb: 帧缓冲对象
 * @return E_OK=成功, 其他=失败
 * @note   若后端空闲，则立即提交 front buffer；若后端忙，则仅记录 pending_commit。
 */
int32_t pixel_framebuffer_commit(struct pixel_framebuffer *fb);

/**
 * @brief  Poll deferred submission
 * @param  fb: 帧缓冲对象
 * @return E_OK=成功, 其他=失败
 * @note   适合放在主循环中周期调用，用于在后端空闲后继续发送挂起帧。
 */
int32_t pixel_framebuffer_poll(struct pixel_framebuffer *fb);

/**
 * @brief  Query backend busy state
 * @param  fb: 帧缓冲对象
 * @return true=忙, false=空闲或不支持查询
 */
bool pixel_framebuffer_is_busy(struct pixel_framebuffer *fb);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __PIXEL_FRAMEBUFFER_H__ */
