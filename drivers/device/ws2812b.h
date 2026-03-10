/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : ws2812b.h
 * @Author       : lxf
 * @Date         : 2026-03-09 18:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-09 21:00:00
 * @Brief        : WS2812B 像素输出驱动
 * @hardware     :
 *               - 适用于单总线级联型 WS2812B/兼容器件
 *               - 推荐底层使用 TIM + PWM + DMA 产生精确波形
 * @features     :
 *               - 接收整帧 RGB888 数据并统一刷新
 *               - 在编码阶段完成 RGB 到 GRB 顺序转换
 *               - 提供全局亮度限制接口，便于限流和夜间模式控制
 *               - 提供忙状态查询，方便上层做异步提交
 * @note         :
 *               - 本驱动故意保持精简，只负责“整帧 -> 时序波形 -> 硬件发送”
 *               - 双缓冲、局部更新、提交策略应放在更高层的像素帧缓冲模块中
 * @usage        :
 *               @code
 *               device_t *ws2812 = device_open("ws2812");
 *               struct ws2812b_info info = { 0 };
 *               uint8_t frame[3] = { 0x20, 0x00, 0x00 };
 *
 *               device_ioctl(ws2812, IOCTL_WS2812B_GET_INFO, &info);
 *               device_write(ws2812, frame, 0, info.frame_size);
 *               @endcode
 */

#ifndef __WS2812B_H__
#define __WS2812B_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/
/* 单颗 WS2812B 固定使用 24bit(G,R,B 各 8bit) 表示颜色。 */
#define WS2812B_BITS_PER_LED           24U

/* 当前驱动对外约定的帧格式为 RGB888。 */
#define WS2812B_BYTES_PER_LED          3U

/* 若上层未指定复位时隙，则默认在帧尾补 64 个低电平 slot。 */
#define WS2812B_DEFAULT_RESET_SLOT_NUM 64U

/* 查询设备基础信息，返回 struct ws2812b_info。 */
#define IOCTL_WS2812B_GET_INFO         (IOCTL_USER_START + 0x00)

/* 设置全局亮度，参数为 uint8_t*，范围 0~255。 */
#define IOCTL_WS2812B_SET_BRIGHTNESS   (IOCTL_USER_START + 0x01)

/* 读取当前全局亮度，参数为 uint8_t*。 */
#define IOCTL_WS2812B_GET_BRIGHTNESS   (IOCTL_USER_START + 0x02)

/* 查询底层是否仍在发送，参数为 bool*。 */
#define IOCTL_WS2812B_IS_BUSY          (IOCTL_USER_START + 0x03)
/*---------- type define ----------*/
/**
 * @brief  WS2812B frame information
 * @note   该结构体用于向上层暴露整帧输出的尺寸信息，便于上层分配像素缓存。
 */
struct ws2812b_info {
    uint16_t led_num;        /* 灯珠数量。 */
    uint8_t bytes_per_pixel; /* 单像素字节数，当前固定为 3(RGB888)。 */
    uint32_t frame_size;     /* 一整帧需要的字节数。 */
};

/**
 * @brief  WS2812B driver describe
 * @note   由 BSP 在静态设备注册时提供。驱动层不关心具体定时器和 DMA 细节，
 *         只要求 BSP 提供初始化、启动发送、停止发送和忙状态查询能力。
 */
struct ws2812b_describe {
    uint16_t led_num;        /* 灯珠数量。 */
    uint8_t brightness;      /* 全局亮度缩放系数，范围 0~255。 */
    uint16_t period_cnt;     /* 单 bit 总周期对应的定时器计数值。 */
    uint16_t t0h_cnt;        /* 发送 bit0 时高电平宽度对应的计数值。 */
    uint16_t t1h_cnt;        /* 发送 bit1 时高电平宽度对应的计数值。 */
    uint16_t reset_slot_num; /* 帧尾复位低电平 slot 数。 */

    struct {
        bool (*init)(void);
        void (*deinit)(void);
        int32_t (*start_transfer)(uint16_t *buf, uint32_t len);
        int32_t (*stop_transfer)(void);
        bool (*is_busy)(void);
    } ops;                   /* 板级硬件操作集合。 */

    struct {
        uint16_t *encode_buf;
        uint32_t encode_len;
    } priv;                  /* 驱动私有编码缓存。 */
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __WS2812B_H__ */
