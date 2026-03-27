/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : tle5012b_sensor.c
 * @Author       : Codex
 * @Date         : 2026-03-25 11:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-26 11:51:36
 * @Brief        : TLE5012B 磁编码器协议驱动实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "tle5012b_sensor.h"
#include "spi_bus.h"
#include <string.h>
/*---------- macro ----------*/
#define TLE5012B_REG_STAT             0x00U
#define TLE5012B_REG_AVAL             0x02U
#define TLE5012B_CMD_READ_MASK        0x8000U
#define TLE5012B_CMD_WORD_COUNT_ONE   0x0001U
#define TLE5012B_SEGMENT_DELAY_US     1U
#define TLE5012B_SAFETY_STAT_SHIFT    12U
#define TLE5012B_SAFETY_RESP_SHIFT    8U
#define TLE5012B_SAFETY_NIBBLE_MASK   0x0FU
#define TLE5012B_SAFETY_CRC_MASK      0x00FFU
#define TLE5012B_SAFETY_STAT_ERR_MASK 0x4000U
#define TLE5012B_SAFETY_STAT_ACC_MASK 0x2000U
#define TLE5012B_SAFETY_STAT_ANG_MASK 0x1000U
#define TLE5012B_ANGLE_RAW_MASK       0x7FFFU
#define TLE5012B_ANGLE_FULL_SCALE     32768.0f
#define TLE5012B_CRC8_SEED            0xFFU
#define TLE5012B_CRC8_XOR_OUT         0xFFU
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static uint16_t _tle5012b_build_read_cmd(uint8_t reg_addr, uint8_t word_count);
static int32_t _tle5012b_sensor_read_reg16(struct tle5012b_sensor *sensor,
                                           uint8_t reg_addr,
                                           uint16_t *data_word,
                                           uint16_t *safety_word);
static uint8_t _tle5012b_get_safety_stat(uint16_t safety_word);
static uint8_t _tle5012b_get_safety_resp(uint16_t safety_word);
static uint8_t _tle5012b_get_safety_crc(uint16_t safety_word);
static uint8_t _tle5012b_crc8_calc_read(uint16_t cmd_word, const uint16_t *data_words, uint16_t data_length);
static int32_t
_tle5012b_check_safety(uint16_t cmd_word, const uint16_t *data_words, uint16_t data_length, uint16_t safety_word);
/*---------- variable ----------*/
static const uint8_t s_tle5012b_crc8_table[256] = {
    0x00U, 0x1DU, 0x3AU, 0x27U, 0x74U, 0x69U, 0x4EU, 0x53U, 0xE8U, 0xF5U, 0xD2U, 0xCFU, 0x9CU, 0x81U, 0xA6U, 0xBBU,
    0xCDU, 0xD0U, 0xF7U, 0xEAU, 0xB9U, 0xA4U, 0x83U, 0x9EU, 0x25U, 0x38U, 0x1FU, 0x02U, 0x51U, 0x4CU, 0x6BU, 0x76U,
    0x87U, 0x9AU, 0xBDU, 0xA0U, 0xF3U, 0xEEU, 0xC9U, 0xD4U, 0x6FU, 0x72U, 0x55U, 0x48U, 0x1BU, 0x06U, 0x21U, 0x3CU,
    0x4AU, 0x57U, 0x70U, 0x6DU, 0x3EU, 0x23U, 0x04U, 0x19U, 0xA2U, 0xBFU, 0x98U, 0x85U, 0xD6U, 0xCBU, 0xECU, 0xF1U,
    0x13U, 0x0EU, 0x29U, 0x34U, 0x67U, 0x7AU, 0x5DU, 0x40U, 0xFBU, 0xE6U, 0xC1U, 0xDCU, 0x8FU, 0x92U, 0xB5U, 0xA8U,
    0xDEU, 0xC3U, 0xE4U, 0xF9U, 0xAAU, 0xB7U, 0x90U, 0x8DU, 0x36U, 0x2BU, 0x0CU, 0x11U, 0x42U, 0x5FU, 0x78U, 0x65U,
    0x94U, 0x89U, 0xAEU, 0xB3U, 0xE0U, 0xFDU, 0xDAU, 0xC7U, 0x7CU, 0x61U, 0x46U, 0x5BU, 0x08U, 0x15U, 0x32U, 0x2FU,
    0x59U, 0x44U, 0x63U, 0x7EU, 0x2DU, 0x30U, 0x17U, 0x0AU, 0xB1U, 0xACU, 0x8BU, 0x96U, 0xC5U, 0xD8U, 0xFFU, 0xE2U,
    0x26U, 0x3BU, 0x1CU, 0x01U, 0x52U, 0x4FU, 0x68U, 0x75U, 0xCEU, 0xD3U, 0xF4U, 0xE9U, 0xBAU, 0xA7U, 0x80U, 0x9DU,
    0xEBU, 0xF6U, 0xD1U, 0xCCU, 0x9FU, 0x82U, 0xA5U, 0xB8U, 0x03U, 0x1EU, 0x39U, 0x24U, 0x77U, 0x6AU, 0x4DU, 0x50U,
    0xA1U, 0xBCU, 0x9BU, 0x86U, 0xD5U, 0xC8U, 0xEFU, 0xF2U, 0x49U, 0x54U, 0x73U, 0x6EU, 0x3DU, 0x20U, 0x07U, 0x1AU,
    0x6CU, 0x71U, 0x56U, 0x4BU, 0x18U, 0x05U, 0x22U, 0x3FU, 0x84U, 0x99U, 0xBEU, 0xA3U, 0xF0U, 0xEDU, 0xCAU, 0xD7U,
    0x35U, 0x28U, 0x0FU, 0x12U, 0x41U, 0x5CU, 0x7BU, 0x66U, 0xDDU, 0xC0U, 0xE7U, 0xFAU, 0xA9U, 0xB4U, 0x93U, 0x8EU,
    0xF8U, 0xE5U, 0xC2U, 0xDFU, 0x8CU, 0x91U, 0xB6U, 0xABU, 0x10U, 0x0DU, 0x2AU, 0x37U, 0x64U, 0x79U, 0x5EU, 0x43U,
    0xB2U, 0xAFU, 0x88U, 0x95U, 0xC6U, 0xDBU, 0xFCU, 0xE1U, 0x5AU, 0x47U, 0x60U, 0x7DU, 0x2EU, 0x33U, 0x14U, 0x09U,
    0x7FU, 0x62U, 0x45U, 0x58U, 0x0BU, 0x16U, 0x31U, 0x2CU, 0x97U, 0x8AU, 0xADU, 0xB0U, 0xE3U, 0xFEU, 0xD9U, 0xC4U,
};
/*---------- function ----------*/
static inline uint16_t _tle5012b_build_read_cmd(uint8_t reg_addr, uint8_t word_count)
{
    return (uint16_t)(TLE5012B_CMD_READ_MASK | ((uint16_t)(reg_addr & 0x3FU) << 4U) | (word_count & 0x0FU));
}

static inline uint8_t _tle5012b_get_safety_stat(uint16_t safety_word)
{
    return (uint8_t)((safety_word >> TLE5012B_SAFETY_STAT_SHIFT) & TLE5012B_SAFETY_NIBBLE_MASK);
}

static inline uint8_t _tle5012b_get_safety_resp(uint16_t safety_word)
{
    return (uint8_t)((safety_word >> TLE5012B_SAFETY_RESP_SHIFT) & TLE5012B_SAFETY_NIBBLE_MASK);
}

static inline uint8_t _tle5012b_get_safety_crc(uint16_t safety_word)
{
    return (uint8_t)(safety_word & TLE5012B_SAFETY_CRC_MASK);
}

static uint8_t _tle5012b_crc8_calc_read(uint16_t cmd_word, const uint16_t *data_words, uint16_t data_length)
{
    uint8_t crc = TLE5012B_CRC8_SEED;
    uint16_t data_index = 0U;

    if ((data_words == NULL) || (data_length == 0U)) {
        return 0U;
    }

    /* 对齐官方 crcCalc：CRC 只覆盖 command 和 data words，不包含 safety word。 */
    crc = s_tle5012b_crc8_table[crc ^ (uint8_t)(cmd_word >> 8U)];
    crc = s_tle5012b_crc8_table[crc ^ (uint8_t)cmd_word];
    for (data_index = 0U; data_index < data_length; data_index++) {
        crc = s_tle5012b_crc8_table[crc ^ (uint8_t)(data_words[data_index] >> 8U)];
        crc = s_tle5012b_crc8_table[crc ^ (uint8_t)data_words[data_index]];
    }

    return (uint8_t)(crc ^ TLE5012B_CRC8_XOR_OUT);
}

static int32_t
_tle5012b_check_safety(uint16_t cmd_word, const uint16_t *data_words, uint16_t data_length, uint16_t safety_word)
{
    uint8_t stat = 0U;
    uint8_t resp = 0U;
    uint8_t crc_expected = 0U;
    uint8_t crc_received = 0U;
    int32_t check_ret = E_OK;

    (void)_tle5012b_get_safety_resp(safety_word);

    stat = _tle5012b_get_safety_stat(safety_word);
    resp = _tle5012b_get_safety_resp(safety_word);
    crc_received = _tle5012b_get_safety_crc(safety_word);
    crc_expected = _tle5012b_crc8_calc_read(cmd_word, data_words, data_length);

    (void)resp;

    /* 对齐官方 checkSafety：优先给出具体状态位错误，再检查 CRC。 */
    if ((safety_word & TLE5012B_SAFETY_STAT_ERR_MASK) == 0U) {
        check_ret = TLE5012B_E_SYSTEM;
    } else if ((safety_word & TLE5012B_SAFETY_STAT_ACC_MASK) == 0U) {
        check_ret = TLE5012B_E_INTERFACE;
    } else if ((safety_word & TLE5012B_SAFETY_STAT_ANG_MASK) == 0U) {
        check_ret = TLE5012B_E_INVALID_ANGLE;
    } else if (crc_expected != crc_received) {
        check_ret = E_WRONG_CRC;
    }

    (void)stat;

    return check_ret;
}

int32_t tle5012b_sensor_init(struct tle5012b_sensor *sensor, device_t *spi_dev)
{
    int32_t init_ret = E_OK;
    uint16_t stat_word = 0U;
    uint16_t safety_word = 0U;

    if ((sensor == NULL) || (spi_dev == NULL)) {
        return E_WRONG_ARGS;
    }

    memset(sensor, 0, sizeof(*sensor));
    sensor->spi_dev = spi_dev;

    init_ret = _tle5012b_sensor_read_reg16(sensor, TLE5012B_REG_STAT, &stat_word, &safety_word);
    if (init_ret != E_OK) {
        sensor->spi_dev = NULL;
        return init_ret;
    }

    (void)stat_word;
    (void)safety_word;

    return E_OK;
}

static int32_t _tle5012b_sensor_read_reg16(struct tle5012b_sensor *sensor,
                                           uint8_t reg_addr,
                                           uint16_t *data_word,
                                           uint16_t *safety_word)
{
    uint16_t cmd_word = 0U;
    uint16_t resp_words[2] = { 0 };
    struct spi_bus_msg msgs[2] = { 0 };
    struct spi_bus_xfer xfer = { 0 };
    int32_t check_ret = E_OK;

    if ((sensor == NULL) || (sensor->spi_dev == NULL) || (data_word == NULL) || (safety_word == NULL)) {
        return E_WRONG_ARGS;
    }

    cmd_word = _tle5012b_build_read_cmd(reg_addr, TLE5012B_CMD_WORD_COUNT_ONE);

    msgs[0].tx_buf = &cmd_word;
    msgs[0].len = 1U;
    msgs[0].bits_per_word = 16U;
    msgs[0].flags = SPI_BUS_MSG_FLAG_WRITE | SPI_BUS_MSG_FLAG_KEEP_CS;
    msgs[0].delay_us = TLE5012B_SEGMENT_DELAY_US;

    msgs[1].rx_buf = resp_words;
    msgs[1].len = 2U;
    msgs[1].bits_per_word = 16U;
    msgs[1].flags = SPI_BUS_MSG_FLAG_READ;
    msgs[1].delay_us = 0U;

    xfer.msgs = msgs;
    xfer.number = ARRAY_SIZE(msgs);

    if (device_ioctl(sensor->spi_dev, IOCTL_SPIBUS_XFER, &xfer) != E_OK) {
        return E_ERROR;
    }

    *data_word = resp_words[0];
    *safety_word = resp_words[1];

    check_ret = _tle5012b_check_safety(cmd_word, resp_words, 1U, resp_words[1]);

    return check_ret;
}

int32_t tle5012b_sensor_read_angle(struct tle5012b_sensor *sensor, struct tle5012b_angle_data *angle_data)
{
    uint16_t raw_word = 0U;
    uint16_t safety_word = 0U;
    int32_t read_ret = E_OK;

    if ((sensor == NULL) || (angle_data == NULL)) {
        return E_WRONG_ARGS;
    }

    memset(angle_data, 0, sizeof(*angle_data));

    read_ret = _tle5012b_sensor_read_reg16(sensor, TLE5012B_REG_AVAL, &raw_word, &safety_word);
    if (read_ret != E_OK) {
        return read_ret;
    }

    angle_data->mechanical_angle_deg =
        ((float)(raw_word & TLE5012B_ANGLE_RAW_MASK) * 360.0f) / TLE5012B_ANGLE_FULL_SCALE;

    return read_ret;
}
/*---------- end of file ----------*/
