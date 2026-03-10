/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : pixel_framebuffer.c
 * @Author       : lxf
 * @Date         : 2026-03-09 18:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-09 21:00:00
 * @Brief        : 通用像素帧缓冲工具实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "pixel_framebuffer.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  Submit current front buffer to output backend
 * @param  fb: 帧缓冲对象
 * @return E_OK=成功, 其他=失败
 */
static int32_t __pixel_framebuffer_submit(struct pixel_framebuffer *fb);

/**
 * @brief  Swap front/back buffer pointers
 * @param  fb: 帧缓冲对象
 */
static void __pixel_framebuffer_swap_buffers(struct pixel_framebuffer *fb);
/*---------- variable ----------*/
/*---------- function ----------*/
int32_t pixel_framebuffer_init(struct pixel_framebuffer *fb,
                               uint16_t pixel_num,
                               uint8_t bytes_per_pixel,
                               const struct pixel_framebuffer_output_ops *ops,
                               void *ops_ctx)
{
    if (!fb || !ops || !ops->submit || (pixel_num == 0U) || (bytes_per_pixel == 0U)) {
        return E_WRONG_ARGS;
    }

    memset(fb, 0, sizeof(*fb));
    fb->pixel_num = pixel_num;
    fb->bytes_per_pixel = bytes_per_pixel;
    fb->frame_size = (uint32_t)pixel_num * bytes_per_pixel;
    fb->ops = ops;
    fb->ops_ctx = ops_ctx;

    fb->front_buf = __malloc(fb->frame_size);
    if (!fb->front_buf) {
        return E_NO_MEMORY;
    }

    fb->back_buf = __malloc(fb->frame_size);
    if (!fb->back_buf) {
        __free(fb->front_buf);
        fb->front_buf = NULL;
        return E_NO_MEMORY;
    }

    memset(fb->front_buf, 0, fb->frame_size);
    memset(fb->back_buf, 0, fb->frame_size);
    fb->dirty = true;

    return E_OK;
}

void pixel_framebuffer_deinit(struct pixel_framebuffer *fb)
{
    if (!fb) {
        return;
    }

    if (fb->front_buf) {
        __free(fb->front_buf);
        fb->front_buf = NULL;
    }

    if (fb->back_buf) {
        __free(fb->back_buf);
        fb->back_buf = NULL;
    }

    fb->frame_size = 0U;
    fb->pixel_num = 0U;
    fb->bytes_per_pixel = 0U;
    fb->dirty = false;
    fb->pending_commit = false;
    fb->ops = NULL;
    fb->ops_ctx = NULL;
}

int32_t pixel_framebuffer_get_info(struct pixel_framebuffer *fb, struct pixel_framebuffer_info *info)
{
    if (!fb || !info) {
        return E_POINT_NONE;
    }

    info->pixel_num = fb->pixel_num;
    info->bytes_per_pixel = fb->bytes_per_pixel;
    info->frame_size = fb->frame_size;
    return E_OK;
}

int32_t pixel_framebuffer_get_back_buffer(struct pixel_framebuffer *fb, uint8_t **ppbuf)
{
    if (!fb || !ppbuf || !fb->back_buf) {
        return E_POINT_NONE;
    }

    *ppbuf = fb->back_buf;
    return E_OK;
}

int32_t pixel_framebuffer_mark_dirty(struct pixel_framebuffer *fb)
{
    if (!fb || !fb->back_buf) {
        return E_POINT_NONE;
    }

    fb->dirty = true;
    return E_OK;
}

int32_t pixel_framebuffer_write_region(struct pixel_framebuffer *fb,
                                       uint32_t offset,
                                       const uint8_t *data,
                                       uint32_t len)
{
    if (!fb || !fb->back_buf || !data) {
        return E_POINT_NONE;
    }

    if ((len == 0U) || (offset + len > fb->frame_size)) {
        return E_WRONG_ARGS;
    }

    memcpy(&fb->back_buf[offset], data, len);
    fb->dirty = true;
    return E_OK;
}

int32_t pixel_framebuffer_set_pixel(struct pixel_framebuffer *fb,
                                    uint16_t index,
                                    struct pixel_framebuffer_color color)
{
    uint32_t offset = 0U;

    if (!fb || !fb->back_buf) {
        return E_POINT_NONE;
    }

    if ((fb->bytes_per_pixel < 3U) || (index >= fb->pixel_num)) {
        return E_WRONG_ARGS;
    }

    offset = (uint32_t)index * fb->bytes_per_pixel;
    fb->back_buf[offset] = color.red;
    fb->back_buf[offset + 1U] = color.green;
    fb->back_buf[offset + 2U] = color.blue;

    /* 对于超过 RGB 的扩展字节，当前统一清零，避免残留旧数据。 */
    for (uint8_t i = 3U; i < fb->bytes_per_pixel; i++) {
        fb->back_buf[offset + i] = 0U;
    }

    fb->dirty = true;
    return E_OK;
}

int32_t pixel_framebuffer_fill(struct pixel_framebuffer *fb,
                               uint16_t start,
                               uint16_t count,
                               struct pixel_framebuffer_color color)
{
    uint32_t actual_count = 0U;

    if (!fb || !fb->back_buf) {
        return E_POINT_NONE;
    }

    if ((fb->bytes_per_pixel < 3U) || (start >= fb->pixel_num)) {
        return E_WRONG_ARGS;
    }

    if (count == UINT16_MAX) {
        actual_count = fb->pixel_num - start;
    } else {
        actual_count = count;
    }

    if ((actual_count == 0U) || ((uint32_t)start + actual_count > fb->pixel_num)) {
        return E_WRONG_ARGS;
    }

    for (uint32_t i = 0; i < actual_count; i++) {
        uint32_t offset = ((uint32_t)start + i) * fb->bytes_per_pixel;

        fb->back_buf[offset] = color.red;
        fb->back_buf[offset + 1U] = color.green;
        fb->back_buf[offset + 2U] = color.blue;

        for (uint8_t j = 3U; j < fb->bytes_per_pixel; j++) {
            fb->back_buf[offset + j] = 0U;
        }
    }

    fb->dirty = true;
    return E_OK;
}

int32_t pixel_framebuffer_clear(struct pixel_framebuffer *fb)
{
    if (!fb || !fb->back_buf) {
        return E_POINT_NONE;
    }

    memset(fb->back_buf, 0, fb->frame_size);
    fb->dirty = true;
    return E_OK;
}

int32_t pixel_framebuffer_commit(struct pixel_framebuffer *fb)
{
    if (!fb || !fb->front_buf || !fb->back_buf || !fb->ops || !fb->ops->submit) {
        return E_POINT_NONE;
    }

    if (!fb->dirty) {
        return E_OK;
    }

    __pixel_framebuffer_swap_buffers(fb);
    fb->dirty = false;

    if (pixel_framebuffer_is_busy(fb)) {
        fb->pending_commit = true;
        return E_OK;
    }

    return __pixel_framebuffer_submit(fb);
}

int32_t pixel_framebuffer_poll(struct pixel_framebuffer *fb)
{
    if (!fb) {
        return E_POINT_NONE;
    }

    if (!fb->pending_commit) {
        return E_OK;
    }

    if (pixel_framebuffer_is_busy(fb)) {
        return E_OK;
    }

    return __pixel_framebuffer_submit(fb);
}

bool pixel_framebuffer_is_busy(struct pixel_framebuffer *fb)
{
    if (!fb || !fb->ops || !fb->ops->is_busy) {
        return false;
    }

    return fb->ops->is_busy(fb->ops_ctx);
}

static int32_t __pixel_framebuffer_submit(struct pixel_framebuffer *fb)
{
    int32_t err = 0;

    err = fb->ops->submit(fb->ops_ctx, fb->front_buf, fb->frame_size);
    if (err == E_OK) {
        fb->pending_commit = false;
    } else if (err == E_BUSY) {
        /* 输出后端繁忙时保留 pending_commit，等待主循环下一次 poll 重试。 */
        fb->pending_commit = true;
    }

    return err;
}

static void __pixel_framebuffer_swap_buffers(struct pixel_framebuffer *fb)
{
    uint8_t *buf = fb->front_buf;

    fb->front_buf = fb->back_buf;
    fb->back_buf = buf;
}
/*---------- end of file ----------*/
