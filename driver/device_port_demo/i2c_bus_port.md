```c
/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : bsp_myi2c.c
 * @Author       : lxf
 * @Date         : 2025-05-12 08:41:13
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-12 08:41:15
 * @Brief        : 软件i2c移植接口
 */


/*---------- includes ----------*/
#include "i2c_bus.h"
#include "export.h"
#include "device.h"
#include "main.h"
/*---------- macro ----------*/

#define GPIO_PORT_I2C GPIOA /* GPIO端口 */

#define I2C_SCL_PIN   GPIO_PIN_6 /* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN   GPIO_PIN_5 /* 连接到SDA数据线的GPIO */
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool _init(void);
static void _scl_set(bool on);
static bool _scl_get(void);
static void _sda_set(bool on);
static bool _sda_get(void);
static void _udelay(uint32_t us);
/*---------- variable ----------*/
static i2c_bit_ops_t ops = {
    .init = _init,
    .deinit = NULL,
    .lock = NULL,
    .unlock = NULL,
    .scl_get = _scl_get,
    .scl_set = _scl_set,
    .sda_get = _sda_get,
    .sda_set = _sda_set,
    .udelay = _udelay,
    .delay_us = 5,
};

static i2cbus_describe_t i2cbus = {
    .ops = &ops,
    .retries = 1000,
};
DEVICE_DEFINED(i2c2, i2c_bus, &i2cbus);

/*---------- function ----------*/
static bool _init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pins : PB6 PB7 */
    GPIO_InitStruct.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIO_PORT_I2C, &GPIO_InitStruct);
    return true;
}
static void _scl_set(bool on)
{
    if (on == true) {
        GPIO_PORT_I2C->BSRR = (uint32_t)I2C_SCL_PIN;
    } else {
        GPIO_PORT_I2C->BSRR = (uint32_t)I2C_SCL_PIN << 16U;
    }
}
static bool _scl_get(void)
{
    return ((GPIOC->IDR & GPIO_PIN_14) != 0);
}
static void _sda_set(bool on)
{
    if (on == true) {
        GPIO_PORT_I2C->BSRR = (uint32_t)GPIO_PIN_5;
    } else {
        GPIO_PORT_I2C->BSRR = (uint32_t)GPIO_PIN_5 << 16U;
    }
}
static bool _sda_get(void)
{
    return ((GPIO_PORT_I2C->IDR & GPIO_PIN_5) != 0);
}
static void _udelay(uint32_t us)
{
    __IO uint8_t i;
    for (i = 0; i < 30; i++) {
        ;
    }
}

/*---------- end of file ----------*/



```

