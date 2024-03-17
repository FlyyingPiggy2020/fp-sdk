/*MIT License

Copyright (c) 2023 Lu Xianfan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : drv_esp32_pin.c
 * @Author       : lxf
 * @Date         : 2024-03-16 13:33:24
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-16 13:33:29
 * @Brief        : esp pin driver
 */

#include "fp_sdk.h"
#include "pin.h"
#include "esp_log.h"
/*---------- includes ----------*/

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

// clang-format off
const struct pin_ops _stm32_pin_ops = {
    // stm32_pin_mode, 
    // stm32_pin_write, 
    // stm32_pin_read, 
    // stm32_pin_attach_irq, 
    // stm32_pin_dettach_irq, 
    // stm32_pin_irq_enable, 
    // stm32_pin_get,
};
// clang-format on

/**
 * @brief esp32 export hook
 * @return {*}
 */
void esp32_pin_link_hook(void) {}

int hw_esp32_pin_init(void)
{
    log_i("esp_pin", "hello world");
    return device_pin_register("pin", &_stm32_pin_ops);
}
INIT_APP_EXPORT(hw_esp32_pin_init);

/*---------- end of file ----------*/
