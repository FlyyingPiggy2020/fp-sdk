/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : spi_bus.c
 * @Author       : Codex
 * @Date         : 2026-03-25 11:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-25 11:20:00
 * @Brief        : SPI 总线驱动实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "spi_bus.h"
#include "driver.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _spi_bus_open(driver_t **pdrv);
static void _spi_bus_close(driver_t **pdrv);
static int32_t _spi_bus_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

static int32_t _spi_bus_device_xfer(struct spi_bus_describe *self, void *args);
static fp_err_t _spi_bus_xfer(struct spi_bus_describe *self, struct spi_bus_msg *msgs, uint32_t number);
static uint16_t _spi_bus_shift_word(struct spi_bus_describe *self, const struct spi_bus_msg *msg, uint16_t tx_word);
static uint16_t _spi_bus_get_tx_word(const struct spi_bus_msg *msg, uint16_t index);
static void _spi_bus_set_rx_word(struct spi_bus_msg *msg, uint16_t index, uint16_t value);
static void _spi_bus_delay_half_period(struct spi_bus_describe *self);
static void _spi_bus_set_idle_state(struct spi_bus_describe *self);
/*---------- variable ----------*/
DRIVER_DEFINED(spi_bus, _spi_bus_open, _spi_bus_close, NULL, NULL, _spi_bus_ioctl, NULL);

static struct protocol_callback s_spi_bus_ioctl_cbs[] = {
    { IOCTL_SPIBUS_XFER, _spi_bus_device_xfer },
};
/*---------- function ----------*/
static void _spi_bus_delay_half_period(struct spi_bus_describe *self)
{
    if ((self == NULL) || (self->ops == NULL)) {
        return;
    }

    if (self->config.half_period_us == 0U) {
        return;
    }

    if (self->ops->delay != NULL) {
        self->ops->delay(self, self->config.half_period_us);
    } else {
        delay_us(self->config.half_period_us);
    }
}

static void _spi_bus_set_idle_state(struct spi_bus_describe *self)
{
    if ((self == NULL) || (self->ops == NULL)) {
        return;
    }

    if (self->ops->cs_set != NULL) {
        self->ops->cs_set(self, false);
    }
    if (self->ops->sck_set != NULL) {
        self->ops->sck_set(self, self->config.cpol != 0U);
    }
    if ((self->ops->sda_dir != NULL) && (self->ops->sda_set != NULL)) {
        self->ops->sda_dir(self, true);
        self->ops->sda_set(self, true);
    }
}

static uint16_t _spi_bus_get_tx_word(const struct spi_bus_msg *msg, uint16_t index)
{
    if ((msg == NULL) || (msg->tx_buf == NULL)) {
        return 0U;
    }

    if (msg->bits_per_word <= 8U) {
        return ((const uint8_t *)msg->tx_buf)[index];
    }

    return ((const uint16_t *)msg->tx_buf)[index];
}

static void _spi_bus_set_rx_word(struct spi_bus_msg *msg, uint16_t index, uint16_t value)
{
    if ((msg == NULL) || (msg->rx_buf == NULL)) {
        return;
    }

    if (msg->bits_per_word <= 8U) {
        ((uint8_t *)msg->rx_buf)[index] = (uint8_t)value;
        return;
    }

    ((uint16_t *)msg->rx_buf)[index] = value;
}

static uint16_t _spi_bus_shift_word(struct spi_bus_describe *self, const struct spi_bus_msg *msg, uint16_t tx_word)
{
    uint16_t rx_word = 0U;
    uint8_t bit_count = 0U;
    bool idle_level = false;
    bool leading_level = false;
    bool trailing_level = false;

    if ((self == NULL) || (msg == NULL) || (self->ops == NULL)) {
        return 0U;
    }

    idle_level = (self->config.cpol != 0U);
    leading_level = !idle_level;
    trailing_level = idle_level;
    bit_count = (msg->bits_per_word <= 8U) ? 8U : 16U;

    for (uint8_t bit_index = 0U; bit_index < bit_count; ++bit_index) {
        uint8_t shift = 0U;
        bool out_bit = false;

        shift = (self->config.lsb_first != 0U) ? bit_index : (uint8_t)(bit_count - 1U - bit_index);
        out_bit = ((tx_word >> shift) & 0x01U) != 0U;

        if ((msg->flags & SPI_BUS_MSG_FLAG_WRITE) != 0U) {
            if ((self->config.cpha == 0U) && (self->ops->sda_set != NULL)) {
                self->ops->sda_set(self, out_bit);
            }
        }

        if (self->ops->sck_set != NULL) {
            self->ops->sck_set(self, leading_level);
        }

        if ((msg->flags & SPI_BUS_MSG_FLAG_WRITE) != 0U) {
            if ((self->config.cpha != 0U) && (self->ops->sda_set != NULL)) {
                self->ops->sda_set(self, out_bit);
            }
        }

        _spi_bus_delay_half_period(self);

        if ((msg->flags & SPI_BUS_MSG_FLAG_READ) != 0U) {
            bool in_bit = false;

            if (self->ops->sda_get != NULL) {
                in_bit = self->ops->sda_get(self);
            }

            if (in_bit) {
                rx_word |= (uint16_t)(1U << shift);
            }
        }

        if (self->ops->sck_set != NULL) {
            self->ops->sck_set(self, trailing_level);
        }
        _spi_bus_delay_half_period(self);
    }

    return rx_word;
}

static fp_err_t _spi_bus_xfer(struct spi_bus_describe *self, struct spi_bus_msg *msgs, uint32_t number)
{
    bool cs_is_active = false;
    fp_err_t err = E_WRONG_ARGS;

    do {
        if ((self == NULL) || (msgs == NULL) || (number == 0U) || (self->ops == NULL)) {
            break;
        }

        if (self->ops->xfer != NULL) {
            return self->ops->xfer(self, msgs, number);
        }

        if ((self->ops->cs_set == NULL) || (self->ops->sck_set == NULL) || (self->ops->sda_set == NULL)
            || (self->ops->sda_get == NULL) || (self->ops->sda_dir == NULL)) {
            break;
        }

        if (self->ops->lock != NULL) {
            self->ops->lock(self);
        }

        for (uint32_t msg_index = 0U; msg_index < number; ++msg_index) {
            struct spi_bus_msg *msg = &msgs[msg_index];

            if ((msg->len == 0U) || ((msg->flags & (SPI_BUS_MSG_FLAG_READ | SPI_BUS_MSG_FLAG_WRITE)) == 0U)) {
                err = E_WRONG_ARGS;
                goto exit;
            }

            if ((msg->bits_per_word != 8U) && (msg->bits_per_word != 16U)) {
                err = E_WRONG_ARGS;
                goto exit;
            }

            if (!cs_is_active) {
                self->ops->cs_set(self, true);
                cs_is_active = true;
            }

            if (((msg->flags & SPI_BUS_MSG_FLAG_READ) != 0U) && ((msg->flags & SPI_BUS_MSG_FLAG_WRITE) == 0U)) {
                self->ops->sda_dir(self, false);
            } else {
                self->ops->sda_dir(self, true);
            }

            for (uint16_t word_index = 0U; word_index < msg->len; ++word_index) {
                uint16_t tx_word = _spi_bus_get_tx_word(msg, word_index);
                uint16_t rx_word = _spi_bus_shift_word(self, msg, tx_word);

                if ((msg->flags & SPI_BUS_MSG_FLAG_READ) != 0U) {
                    _spi_bus_set_rx_word(msg, word_index, rx_word);
                }
            }

            if (msg->delay_us > 0U) {
                if (self->ops->delay != NULL) {
                    self->ops->delay(self, msg->delay_us);
                } else {
                    delay_us(msg->delay_us);
                }
            }

            if ((msg->flags & SPI_BUS_MSG_FLAG_KEEP_CS) == 0U) {
                self->ops->cs_set(self, false);
                cs_is_active = false;
            }
        }

        err = E_OK;
    } while (0);

exit:
    if (cs_is_active && (self != NULL) && (self->ops != NULL) && (self->ops->cs_set != NULL)) {
        self->ops->cs_set(self, false);
    }
    if ((self != NULL) && (self->ops != NULL) && (self->ops->unlock != NULL)) {
        self->ops->unlock(self);
    }
    return err;
}

static int32_t _spi_bus_device_xfer(struct spi_bus_describe *self, void *args)
{
    struct spi_bus_xfer *xfer = (struct spi_bus_xfer *)args;

    if ((self == NULL) || (xfer == NULL)) {
        return E_WRONG_ARGS;
    }

    return _spi_bus_xfer(self, xfer->msgs, xfer->number);
}

static int32_t _spi_bus_open(driver_t **pdrv)
{
    struct spi_bus_describe *pdesc = NULL;

    assert(pdrv != NULL);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if ((pdesc == NULL) || (pdesc->ops == NULL) || (pdesc->ops->init == NULL)) {
        return E_WRONG_ARGS;
    }

    if (pdesc->ops->init(pdesc, &pdesc->config) != E_OK) {
        return E_ERROR;
    }

    _spi_bus_set_idle_state(pdesc);

    return E_OK;
}

static void _spi_bus_close(driver_t **pdrv)
{
    struct spi_bus_describe *pdesc = NULL;

    assert(pdrv != NULL);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if ((pdesc == NULL) || (pdesc->ops == NULL)) {
        return;
    }

    _spi_bus_set_idle_state(pdesc);

    if (pdesc->ops->deinit != NULL) {
        pdesc->ops->deinit(pdesc);
    }
}

static int32_t _spi_bus_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    struct spi_bus_describe *pdesc = NULL;
    int32_t (*cb)(struct spi_bus_describe *, void *) = NULL;

    assert(pdrv != NULL);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc == NULL) {
        return E_WRONG_ARGS;
    }

    cb = (int32_t (*)(struct spi_bus_describe *, void *))protocol_callback_find(
        cmd, s_spi_bus_ioctl_cbs, ARRAY_SIZE(s_spi_bus_ioctl_cbs));
    if (cb == NULL) {
        return E_WRONG_ARGS;
    }

    return cb(pdesc, args);
}
/*---------- end of file ----------*/
