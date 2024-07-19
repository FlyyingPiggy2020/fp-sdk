/*
 * Copyright (c) 2024 by Lu Xianfan, Ltd.
 * @FilePath     : fp_si446x.c
 * @Author       : lxf
 * @Date         : 2024-07-09 14:46:31
 * @LastEditors  : lxf 154562451@qq.com
 * @LastEditTime : 2024-07-14 15:57:50
 * @Brief        : si446x driver
 */

/*---------- includes ----------*/

#include "fp_si446x.h"
#include "stddef.h"
/*---------- macro ----------*/

/**
 * @brief si446x common commands
 */
#define SI446X_NOP                  0X00
#define SI446X_PART_INFO            0X01
#define SI446X_FUNC_INFO            0X10
#define SI446X_SET_PROPERTY         0X11
#define SI446X_GET_PROPERTY         0X12
#define SI446X_FIFO_INFO            0X15
#define SI446X_GET_INT_STATUS       0X20
#define SI446X_REQUEST_DEVICE_STATE 0X33
#define SI446X_CHANGE_STATE         0X34
#define SI446X_READ_CMD_BUFF        0X44
#define SI446X_FRR_A_READ           0X50
#define SI446X_FRR_B_READ           0X51
#define SI446X_FRR_C_READ           0X53
#define SI446X_FRR_D_READ           0X57
/*---------- type define ----------*/

typedef int (*fp_si446x_ioctl_cb_t)(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg);
typedef struct _ioctl_cb {
    int cmd;
    fp_si446x_ioctl_cb_t cb;
} _ioctl_cb_t;

/*---------- variable prototype ----------*/
static int _set_frequency(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg);
static int _init(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg);
static int _read_part_info(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg);
static _ioctl_cb_t ioctl_cb[] = {
    { IOCTL_SI446X_INIT, _init },
    { IOCTL_SI446X_SET_FREQUENCY, _set_frequency },
    { IOCTL_SI446X_READ_PART_INFO, _read_part_info },
};
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief get command response foram radio chip
 * @param {fp_si446x_dev_t} si446x_dev
 * @param {unsigned char} *buff
 * @param {unsigned int} len
 * @return {*}
 */
unsigned char _get_resp(fp_si446x_dev_t *si446x_dev, unsigned int len, unsigned char *buff)
{
    unsigned char send_buff[1] = { SI446X_READ_CMD_BUFF };
    unsigned char cts = 0;
    unsigned short errcnt = 2000;
    while (errcnt != 0) {
        si446x_dev->spi_dev->cs(0);
        spi_send_recv_without_cs(si446x_dev->spi_dev, send_buff, NULL, 1);
        spi_send_recv_without_cs(si446x_dev->spi_dev, NULL, &cts, 1);
        if (cts == 0xff) {
            for (unsigned int i = 0; i < len; i++) {
                spi_send_recv_without_cs(si446x_dev->spi_dev, (unsigned char[]){ 0xff }, &buff[i], 1);
            }
            break;
        }
        si446x_dev->spi_dev->cs(1);
        errcnt--;
    }
    if (errcnt == 0) {
        return 0;
    } else {
        return 0xff;
    }
}

/**
 * @brief if radio is idle, cts will be 0xff
 * @param {fp_si446x_dev_t} *si446x_dev
 * @return {*}
 */
unsigned char _wait_cts(fp_si446x_dev_t *si446x_dev)
{
    return _get_resp(si446x_dev, 0, 0);
}

/**
 * @brief send command
 * @param {fp_si446x_dev_t} *si446x_dev
 * @param {unsigned int} len
 * @param {unsigned char} *buff
 * @return {*}
 */
void _send_cmd(fp_si446x_dev_t *si446x_dev, unsigned int len, unsigned char *buff)
{
    do {
        if (_wait_cts(si446x_dev) != 0xff) {
            break;
        }
        si446x_dev->spi_dev->cs(0);
        for (unsigned int i; i < len; i++) {
            spi_send_recv_without_cs(si446x_dev->spi_dev, buff, NULL, 1);
        }
        si446x_dev->spi_dev->cs(1);
    } while (0);
}

static int _set_frequency(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg)
{
    return 0;
}

/**
 * @brief read part info
 * @param {fp_si446x_dev_t} *si446x_dev
 * @param {unsigned int} cmd
 * @param {void} *arg
 * @return {*}
 */
static int _read_part_info(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg)
{
    int retval = 0x00;
    _send_cmd(si446x_dev, 1, (unsigned char[]){ SI446X_PART_INFO });
    retval = _get_resp(si446x_dev, 8, (unsigned char *)&si446x_dev->info.part_info);

    return retval;
}
static fp_si446x_ioctl_cb_t _find_ioctl_cb(unsigned int cmd)
{
    for (int i = 0; i < sizeof(ioctl_cb) / sizeof(_ioctl_cb_t); i++) {
        if (ioctl_cb[i].cmd == cmd && ioctl_cb[i].cb != NULL) {
            return ioctl_cb[i].cb;
        }
    }
    return NULL;
}

static int _init(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg)
{
    si446x_dev->snd(0);
    si446x_dev->spi_dev->cs(1);
    /* power up */
    si446x_dev->snd(1);
    si446x_dev->delay_ms(10);
    si446x_dev->snd(0);
    si446x_dev->delay_ms(10);

    /* read part info */
    _read_part_info(si446x_dev, cmd, arg);
    return 0;
}

void si446x_ioctl(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg)
{
    fp_si446x_ioctl_cb_t cb = _find_ioctl_cb(cmd);
    if (cb != NULL) {
        cb(si446x_dev, cmd, arg);
    }
}
/*---------- end of file ----------*/
