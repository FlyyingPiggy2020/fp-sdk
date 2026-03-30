/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : tle5012b_sensor.h
 * @Author       : Codex
 * @Date         : 2026-03-25 11:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-25 11:20:00
 * @Brief        : TLE5012B 磁编码器协议驱动
 */

#ifndef __TLE5012B_SENSOR_H__
#define __TLE5012B_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "device.h"
#include <stdint.h>
/*---------- macro ----------*/
#define TLE5012B_E_SYSTEM              (-101)
#define TLE5012B_E_INTERFACE           (-102)
#define TLE5012B_E_INVALID_ANGLE       (-103)
/*---------- type define ----------*/
struct tle5012b_angle_data {
    float mechanical_angle_deg;
};

struct tle5012b_sensor {
    device_t *spi_dev;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
int32_t tle5012b_sensor_init(struct tle5012b_sensor *sensor, device_t *spi_dev);
int32_t tle5012b_sensor_read_angle(struct tle5012b_sensor *sensor, struct tle5012b_angle_data *angle_data);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __TLE5012B_SENSOR_H__ */
