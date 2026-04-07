/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : spi_bus.h
 * @Author       : Codex
 * @Date         : 2026-03-25 11:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-25 11:20:00
 * @Brief        : SPI 总线驱动接口
 */

#ifndef __SPI_BUS_H__
#define __SPI_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "define.h"
#include "device.h"
#include <stdbool.h>
#include <stdint.h>
/*---------- macro ----------*/
#define IOCTL_SPIBUS_XFER        (IOCTL_USER_START + 0x00)

#define SPI_BUS_MSG_FLAG_READ    (1U << 0)
#define SPI_BUS_MSG_FLAG_WRITE   (1U << 1)
#define SPI_BUS_MSG_FLAG_KEEP_CS (1U << 2)
/*---------- type define ----------*/
struct spi_bus_describe;

struct spi_bus_msg {
    const void *tx_buf;
    void *rx_buf;
    uint16_t len;
    uint8_t bits_per_word;
    uint8_t flags;
    uint16_t delay_us;
};

struct spi_bus_xfer {
    struct spi_bus_msg *msgs;
    uint32_t number;
};

struct spi_bus_config {
    uint8_t cpol;
    uint8_t cpha;
    uint8_t cs_active_high;
    uint8_t lsb_first;
    uint16_t half_period_us;
};

struct spi_bus_ops {
    fp_err_t (*init)(struct spi_bus_describe *self, struct spi_bus_config *cfg);
    void (*deinit)(struct spi_bus_describe *self);

    void (*cs_set)(struct spi_bus_describe *self, bool active);
    void (*sck_set)(struct spi_bus_describe *self, bool high);
    void (*sda_set)(struct spi_bus_describe *self, bool high);
    bool (*sda_get)(struct spi_bus_describe *self);
    void (*sda_dir)(struct spi_bus_describe *self, bool output);
    void (*delay)(struct spi_bus_describe *self, uint32_t us);
    void (*lock)(struct spi_bus_describe *self);
    void (*unlock)(struct spi_bus_describe *self);

    fp_err_t (*xfer)(struct spi_bus_describe *self, struct spi_bus_msg *msgs, uint32_t number);
};

struct spi_bus_describe {
    struct spi_bus_config config;
    struct spi_bus_ops *ops;
    void *hw_handle;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __SPI_BUS_H__ */
