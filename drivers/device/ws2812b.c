/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : ws2812b.c
 * @Author       : lxf
 * @Date         : 2026-03-09 18:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-09 21:00:00
 * @Brief        : WS2812B 像素输出驱动实现
 * @transmit_flow:
 *               - 上层通过 device_write() 提交一整帧 RGB888 数据
 *               - 驱动将 RGB 顺序重排为 WS2812B 所需的 G-R-B 位流
 *               - 驱动按 bit 展开为 PWM compare 序列并追加复位低电平时隙
 *               - BSP 通过 TIM + DMA 将 compare 序列送到实际输出引脚
 */

/*---------- includes ----------*/
#include "options.h"
#include "ws2812b.h"
#include "driver.h"
#include "misc.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  Open ws2812b device
 * @param  pdrv: 设备驱动对象指针地址
 * @return E_OK=成功, 其他=失败
 */
static int32_t ws2812b_open(driver_t **pdrv);

/**
 * @brief  Close ws2812b device
 * @param  pdrv: 设备驱动对象指针地址
 */
static void ws2812b_close(driver_t **pdrv);

/**
 * @brief  Submit one RGB frame to ws2812b
 * @param  pdrv: 设备驱动对象指针地址
 * @param  buf: RGB888 帧数据
 * @param  addition: 保留参数，当前未使用
 * @param  len: 帧长度，必须等于 led_num * 3
 * @return E_OK=成功, 其他=失败
 */
static int32_t ws2812b_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);

/**
 * @brief  Process ws2812b ioctl command
 * @param  pdrv: 设备驱动对象指针地址
 * @param  cmd: ioctl 命令
 * @param  args: 命令参数
 * @return E_OK=成功, 其他=失败
 */
static int32_t ws2812b_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

static int32_t _ioctl_get_info(struct ws2812b_describe *pdesc, void *args);
static int32_t _ioctl_set_brightness(struct ws2812b_describe *pdesc, void *args);
static int32_t _ioctl_get_brightness(struct ws2812b_describe *pdesc, void *args);
static int32_t _ioctl_is_busy(struct ws2812b_describe *pdesc, void *args);

/**
 * @brief  Encode one RGB frame into PWM compare sequence
 * @param  pdesc: 驱动描述对象
 * @param  frame: RGB888 输入帧
 * @param  len: 输入长度
 * @return E_OK=成功, 其他=失败
 */
static int32_t __ws2812b_encode_frame(struct ws2812b_describe *pdesc, const uint8_t *frame, uint32_t len);

/**
 * @brief  Encode one byte into 8 PWM slots
 * @param  pdesc: 驱动描述对象
 * @param  buf: 输出缓冲起始地址
 * @param  data: 待编码字节
 */
static void __ws2812b_encode_byte(struct ws2812b_describe *pdesc, uint16_t *buf, uint8_t data);

/**
 * @brief  Apply global brightness scaling
 * @param  value: 原始颜色分量
 * @param  brightness: 全局亮度 0~255
 * @return 缩放后的颜色分量
 */
static uint8_t __ws2812b_apply_brightness(uint8_t value, uint8_t brightness);
/*---------- variable ----------*/
DRIVER_DEFINED(ws2812b, ws2812b_open, ws2812b_close, ws2812b_write, NULL, ws2812b_ioctl, NULL);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_WS2812B_GET_INFO, _ioctl_get_info },
    { IOCTL_WS2812B_SET_BRIGHTNESS, _ioctl_set_brightness },
    { IOCTL_WS2812B_GET_BRIGHTNESS, _ioctl_get_brightness },
    { IOCTL_WS2812B_IS_BUSY, _ioctl_is_busy },
};
/*---------- function ----------*/
static int32_t ws2812b_open(driver_t **pdrv)
{
    struct ws2812b_describe *pdesc = NULL;
    int32_t err = E_WRONG_ARGS;
    bool is_inited = false;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            err = E_POINT_NONE;
            break;
        }

        if ((pdesc->led_num == 0U) || (pdesc->period_cnt == 0U) || (pdesc->t0h_cnt == 0U) || (pdesc->t1h_cnt == 0U)) {
            break;
        }

        if ((pdesc->t0h_cnt >= pdesc->period_cnt) || (pdesc->t1h_cnt >= pdesc->period_cnt)) {
            break;
        }

        if (!pdesc->ops.init || !pdesc->ops.start_transfer) {
            err = E_POINT_NONE;
            break;
        }

        if (pdesc->reset_slot_num == 0U) {
            pdesc->reset_slot_num = WS2812B_DEFAULT_RESET_SLOT_NUM;
        }

        pdesc->priv.encode_len = (uint32_t)pdesc->led_num * WS2812B_BITS_PER_LED + pdesc->reset_slot_num;
        pdesc->priv.encode_buf = __malloc(sizeof(uint16_t) * pdesc->priv.encode_len);
        if (!pdesc->priv.encode_buf) {
            err = E_NO_MEMORY;
            break;
        }
        memset(pdesc->priv.encode_buf, 0, sizeof(uint16_t) * pdesc->priv.encode_len);

        if (!pdesc->ops.init()) {
            err = E_ERROR;
            break;
        }
        is_inited = true;

        err = E_OK;
    } while (0);

    if (err != E_OK) {
        if (is_inited && pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }

        if (pdesc && pdesc->priv.encode_buf) {
            __free(pdesc->priv.encode_buf);
            pdesc->priv.encode_buf = NULL;
        }

        if (pdesc) {
            pdesc->priv.encode_len = 0U;
        }
    }

    return err;
}

static void ws2812b_close(driver_t **pdrv)
{
    struct ws2812b_describe *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (!pdesc) {
        return;
    }

    if (pdesc->ops.is_busy && pdesc->ops.is_busy()) {
        if (pdesc->ops.stop_transfer) {
            pdesc->ops.stop_transfer();
        }
    }

    if (pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }

    if (pdesc->priv.encode_buf) {
        __free(pdesc->priv.encode_buf);
        pdesc->priv.encode_buf = NULL;
    }

    pdesc->priv.encode_len = 0U;
}

static int32_t ws2812b_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    struct ws2812b_describe *pdesc = NULL;

    (void)addition;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (!pdesc || !buf || !pdesc->priv.encode_buf) {
        return E_POINT_NONE;
    }

    if (pdesc->ops.is_busy && pdesc->ops.is_busy()) {
        return E_BUSY;
    }

    if (__ws2812b_encode_frame(pdesc, (const uint8_t *)buf, len) != E_OK) {
        return E_WRONG_ARGS;
    }

    return pdesc->ops.start_transfer(pdesc->priv.encode_buf, pdesc->priv.encode_len);
}

static int32_t ws2812b_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    struct ws2812b_describe *pdesc = NULL;
    int32_t err = E_WRONG_ARGS;
    int32_t (*cb)(struct ws2812b_describe *, void *) = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            err = E_POINT_NONE;
            break;
        }

        cb = (int32_t (*)(struct ws2812b_describe *, void *))protocol_callback_find(
            cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }

        err = cb(pdesc, args);
    } while (0);

    return err;
}

/**
 * @brief  Read fixed frame information
 * @param  pdesc: 驱动描述对象
 * @param  args: struct ws2812b_info* 输出参数
 * @return E_OK=成功, 其他=失败
 */
static int32_t _ioctl_get_info(struct ws2812b_describe *pdesc, void *args)
{
    struct ws2812b_info *info = args;

    if (!pdesc || !info) {
        return E_POINT_NONE;
    }

    info->led_num = pdesc->led_num;
    info->bytes_per_pixel = WS2812B_BYTES_PER_LED;
    info->frame_size = (uint32_t)pdesc->led_num * WS2812B_BYTES_PER_LED;
    return E_OK;
}

/**
 * @brief  Update global brightness limit
 * @param  pdesc: 驱动描述对象
 * @param  args: uint8_t* 亮度参数
 * @return E_OK=成功, 其他=失败
 */
static int32_t _ioctl_set_brightness(struct ws2812b_describe *pdesc, void *args)
{
    uint8_t *brightness = args;

    if (!pdesc || !brightness) {
        return E_POINT_NONE;
    }

    pdesc->brightness = *brightness;
    return E_OK;
}

/**
 * @brief  Query current global brightness
 * @param  pdesc: 驱动描述对象
 * @param  args: uint8_t* 输出参数
 * @return E_OK=成功, 其他=失败
 */
static int32_t _ioctl_get_brightness(struct ws2812b_describe *pdesc, void *args)
{
    uint8_t *brightness = args;

    if (!pdesc || !brightness) {
        return E_POINT_NONE;
    }

    *brightness = pdesc->brightness;
    return E_OK;
}

/**
 * @brief  Query hardware busy state
 * @param  pdesc: 驱动描述对象
 * @param  args: bool* 输出参数
 * @return E_OK=成功, 其他=失败
 */
static int32_t _ioctl_is_busy(struct ws2812b_describe *pdesc, void *args)
{
    bool *is_busy = args;

    if (!pdesc || !is_busy) {
        return E_POINT_NONE;
    }

    if (pdesc->ops.is_busy) {
        *is_busy = pdesc->ops.is_busy();
    } else {
        *is_busy = false;
    }

    return E_OK;
}

static int32_t __ws2812b_encode_frame(struct ws2812b_describe *pdesc, const uint8_t *frame, uint32_t len)
{
    uint32_t offset = 0U;
    uint32_t frame_size = 0U;

    if (!pdesc || !frame || !pdesc->priv.encode_buf) {
        return E_POINT_NONE;
    }

    frame_size = (uint32_t)pdesc->led_num * WS2812B_BYTES_PER_LED;
    if (len != frame_size) {
        return E_WRONG_ARGS;
    }

    for (uint16_t i = 0; i < pdesc->led_num; i++) {
        uint32_t base = (uint32_t)i * WS2812B_BYTES_PER_LED;
        uint8_t red = __ws2812b_apply_brightness(frame[base], pdesc->brightness);
        uint8_t green = __ws2812b_apply_brightness(frame[base + 1U], pdesc->brightness);
        uint8_t blue = __ws2812b_apply_brightness(frame[base + 2U], pdesc->brightness);

        /* WS2812B 按 G-R-B 顺序发送，因此这里按顺序展开三个颜色分量。 */
        __ws2812b_encode_byte(pdesc, &pdesc->priv.encode_buf[offset], green);
        offset += 8U;
        __ws2812b_encode_byte(pdesc, &pdesc->priv.encode_buf[offset], red);
        offset += 8U;
        __ws2812b_encode_byte(pdesc, &pdesc->priv.encode_buf[offset], blue);
        offset += 8U;
    }

    /* 帧尾补零，用于拉出复位低电平时间。 */
    while (offset < pdesc->priv.encode_len) {
        pdesc->priv.encode_buf[offset++] = 0U;
    }

    return E_OK;
}

static void __ws2812b_encode_byte(struct ws2812b_describe *pdesc, uint16_t *buf, uint8_t data)
{
    for (uint8_t i = 0; i < 8U; i++) {
        if ((data & (0x80U >> i)) != 0U) {
            buf[i] = pdesc->t1h_cnt;
        } else {
            buf[i] = pdesc->t0h_cnt;
        }
    }
}

static uint8_t __ws2812b_apply_brightness(uint8_t value, uint8_t brightness)
{
    return (uint8_t)(((uint16_t)value * brightness + 127U) / 255U);
}
/*---------- end of file ----------*/
