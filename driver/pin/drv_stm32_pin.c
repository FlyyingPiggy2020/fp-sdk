/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : drv_stm32_pin.c
 * @Author       : lxf
 * @Date         : 2024-02-23 08:38:51
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-23 11:47:28
 * @Brief        : stm32 gpio driver
 */

/*---------- includes ----------*/

#include "main.h"
#include "inc/pin.h"
/*---------- macro ----------*/

#define PIN_NUM(port, no) (((((port)&0xFu) << 4) | ((no)&0xFu)))
#define PIN_PORT(pin)     ((uint8_t)(((pin) >> 4) & 0xFu))
#define PIN_NO(pin)       ((uint8_t)((pin)&0xFu))

#define PIN_STPORT(pin)   ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)    ((uint16_t)(1u << PIN_NO(pin)))

#if defined(GPIOZ)
#define __STM32_PORT_MAX 12u
#elif defined(GPIOK)
#define __STM32_PORT_MAX 11u
#elif defined(GPIOJ)
#define __STM32_PORT_MAX 10u
#elif defined(GPIOI)
#define __STM32_PORT_MAX 9u
#elif defined(GPIOH)
#define __STM32_PORT_MAX 8u
#elif defined(GPIOG)
#define __STM32_PORT_MAX 7u
#elif defined(GPIOF)
#define __STM32_PORT_MAX 6u
#elif defined(GPIOE)
#define __STM32_PORT_MAX 5u
#elif defined(GPIOD)
#define __STM32_PORT_MAX 4u
#elif defined(GPIOC)
#define __STM32_PORT_MAX 3u
#elif defined(GPIOB)
#define __STM32_PORT_MAX 2u
#elif defined(GPIOA)
#define __STM32_PORT_MAX 1u
#else
#define __STM32_PORT_MAX 0u
#error Unsupported STM32 GPIO peripheral.
#endif

#define PIN_STPORT_MAX __STM32_PORT_MAX
/*---------- type define ----------*/
/*---------- variable prototype ----------*/

static void stm32_pin_write(device_t *dev, uint32_t pin, uint8_t value)
{
    GPIO_TypeDef *gpio_port;
    uint16_t gpio_pin;

    if (PIN_PORT(pin) < PIN_STPORT_MAX) {
        gpio_port = PIN_STPORT(pin);
        gpio_pin = PIN_STPIN(pin);

        HAL_GPIO_WritePin(gpio_port, gpio_pin, (GPIO_PinState)value);
    }
}

static int8_t stm32_pin_read(device_t *dev, uint32_t pin)
{
    GPIO_TypeDef *gpio_port;
    uint16_t gpio_pin;
    GPIO_PinState state = PIN_LOW;

    if (PIN_PORT(pin) < PIN_STPORT_MAX) {
        gpio_port = PIN_STPORT(pin);
        gpio_pin = PIN_STPIN(pin);
        state = HAL_GPIO_ReadPin(gpio_port, gpio_pin);
    }

    return (state == GPIO_PIN_RESET) ? PIN_LOW : PIN_HIGH;
}

static void stm32_pin_mode(device_t *dev, uint32_t pin, uint8_t mode)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (PIN_PORT(pin) >= PIN_STPORT_MAX) {
        return;
    }

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin = PIN_STPIN(pin);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (mode == PIN_MODE_OUTPUT) {
        /* output setting */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    } else if (mode == PIN_MODE_INPUT) {
        /* input setting: not pull. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    } else if (mode == PIN_MODE_INPUT_PULLUP) {
        /* input setting: pull up. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    } else if (mode == PIN_MODE_INPUT_PULLDOWN) {
        /* input setting: pull down. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    } else if (mode == PIN_MODE_OUTPUT_OD) {
        /* output setting: od. */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }

    HAL_GPIO_Init(PIN_STPORT(pin), &GPIO_InitStruct);
}

// clang-format off
const struct pin_ops _stm32_pin_ops = {
    stm32_pin_mode, 
    stm32_pin_write, 
    stm32_pin_read, 
    // stm32_pin_attach_irq, 
    // stm32_pin_dettach_irq, 
    // stm32_pin_irq_enable, 
    // stm32_pin_get,
};
// clang-format on

/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

int hw_pin_init(void)
{
    return device_pin_register("pin", &_stm32_pin_ops);
}
INIT_DEVICE_EXPORT(hw_pin_init);
/*---------- end of file ----------*/
