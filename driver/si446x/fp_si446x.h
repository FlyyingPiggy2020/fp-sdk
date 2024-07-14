/*
 * Copyright (c) 2024 by Lu Xianfan, Ltd.
 * @FilePath     : fp_si446x.h
 * @Author       : lxf
 * @Date         : 2024-07-09 14:46:31
 * @LastEditors  : lxf 154562451@qq.com
 * @LastEditTime : 2024-07-13 17:01:29
 * @Brief        : si446x driver
 */

#ifndef __FP_SI446X_H__
#define __FP_SI446X_H__
/*---------- includes ----------*/

#include "../spi/fp_spi.h"
/*---------- macro ----------*/

/**
 * @brief IOCTL command to initialize the SI446X driver.
 *
 * This IOCTL command is used to initialize the SI446X driver.
 * It is typically used to set up the necessary configurations and parameters
 * before using the SI446X device.
 *
 * @note This command should be called before any other SI446X driver functions.
 *
 */
#define IOCTL_SI446X_INIT                        0x00
/**
 * @brief IOCTL command to set the frequency for the SI446X driver.
 *
 * This IOCTL command is used to set the frequency for the SI446X driver.
 * The frequency value should be provided as a parameter when calling the IOCTL function.
 * The frequency value is typically specified in Hz.
 *
 */
#define IOCTL_SI446X_SET_FREQUENCY               0x01
/**
 * @brief IOCTL command to set the baudrate for the SI446X driver.
 *
 * This IOCTL command is used to set the baudrate for the SI446X driver.
 * The frequency value is typically specified in Kbps.
 *
 */
#define IOCTL_SI446X_SET_BAUDRATE                0X02
/**
 * @brief IOCTL command to set the TX_FIFO_ALMOST_EMPTY_EN for the SI446X driver.
 *
 * 0 disable,1 enable
 *
 */
#define IOCTL_SI446X_SET_TX_FIFO_ALMOST_EMPTY_EN 0X03

/**
 * @brief IOTTL command to get si446x chip info
 *
 */
#define IOCTL_SI446X_READ_PART_INFO              0X04
/*---------- type define ----------*/
typedef struct fp_si446x_info {
    struct {
        unsigned char chiprev;
        unsigned char part_h;
        unsigned char part_l;
        unsigned char pbuild;
        unsigned char id_h;
        unsigned char id_l;
        unsigned char customer;
        unsigned char romid;
    } part_info;
} fp_si446x_info_t;

typedef struct fp_si446x_dev {
    fp_spi_dev_t *spi_dev;
    fp_si446x_info_t info;
    void (*snd)(unsigned char state);
    void (*delay_ms)(unsigned int ms);
} fp_si446x_dev_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

void si446x_ioctl(fp_si446x_dev_t *si446x_dev, unsigned int cmd, void *arg);
/*---------- end of file ----------*/
#endif
