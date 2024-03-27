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
 * @FilePath     : drv_esp32_serial.c
 * @Author       : lxf
 * @Date         : 2024-03-16 13:30:38
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-16 13:30:39
 * @Brief        : esp32 serial driver
 */

/*---------- includes ----------*/

#include "serial.h"
#include "driver/uart.h"
#include "esp_log.h"
/*---------- macro ----------*/

#define TAG "esp_serial"
/*---------- type define ----------*/

static struct device_serial _hw_serial0;
static struct device_serial _hw_serial1;
TaskHandle_t serial0_Handle = NULL;
TaskHandle_t serial1_Handle = NULL;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

static void uart0_event_task(void *pvParameters);
static void uart1_event_task(void *pvParameters);
/*---------- variable ----------*/
/*---------- function ----------*/
static fp_size_t esp32_serial_raed(struct device *device, uint8_t *buff, fp_size_t wanted_size)
{
    return 0;
}
static void esp32_serial_write(struct device *device, uint8_t *buff, fp_size_t size)
{
    return;
}

static uart_config_t _get_uart_config(struct serial_config config)
{
    uart_config_t uart_config = {
        .baud_rate = config.baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    switch (config.data_bits) {
        case SERIAL_DATA_5_BITS:
            uart_config.data_bits = UART_DATA_5_BITS;
            break;
        case SERIAL_DATA_6_BITS:
            uart_config.data_bits = UART_DATA_6_BITS;
            break;
        case SERIAL_DATA_7_BITS:
            uart_config.data_bits = UART_DATA_7_BITS;
            break;
        case SERIAL_DATA_8_BITS:
            uart_config.data_bits = UART_DATA_8_BITS;
            break;
        default:
            break;
    }

    switch (config.parity) {
        case SERIAL_PARITY_DISABLE:
            uart_config.parity = UART_PARITY_DISABLE;
            break;
        case SERIAL_PARITY_EVEN:
            uart_config.parity = UART_PARITY_EVEN;
            break;
        case SERIAL_PARITY_ODD:
            uart_config.parity = UART_PARITY_ODD;
            break;
        default:
            break;
    }

    switch (config.stop_bits) {
        case SERIAL_STOP_BITS_1:
            uart_config.stop_bits = UART_STOP_BITS_1;
            break;
        case SERIAL_STOP_BITS_1_5:
            uart_config.stop_bits = UART_STOP_BITS_1_5;
            break;
        case SERIAL_STOP_BITS_2:
            uart_config.stop_bits = UART_STOP_BITS_2;
            break;
        default:
            break;
    }
    return uart_config;
}
static fp_size_t esp32_serial_config(struct device *device, struct serial_config *config)
{
    struct device_serial *serial = (struct device_serial *)device;

    assert(serial->ops != NULL);
    assert((serial->usart_port == 0) || (serial->usart_port == 1));
    uart_config_t uart_config = _get_uart_config(*config);
    serial->config = config;

    uart_driver_install(serial->usart_port, config->rx_buffer_size, config->tx_buffer_size, 20, &serial->event_queue, 0);
    uart_param_config(serial->usart_port, &uart_config);
    uart_set_pin(serial->usart_port, config->tx_io_num, config->rx_io_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    if (serial0_Handle == NULL) {
        xTaskCreate(uart0_event_task, "uart0_event_task", 2048, NULL, 12, &serial0_Handle);
    }
    return 0;
}

// clang-format off
const struct serial_ops _esp32_serial_ops = {
    esp32_serial_raed, 
    esp32_serial_write,
    esp32_serial_config,
};
// clang-format on

void esp32_serial_link_hook(void) {}

int hw_esp32_serial_init(void)
{
    ESP_LOGI(TAG, "esp serial init");
    _hw_serial0.usart_port = 0;
    _hw_serial1.usart_port = 1;

    device_serial_register(&_hw_serial0, "serial0", &_esp32_serial_ops);
    device_serial_register(&_hw_serial1, "serial1", &_esp32_serial_ops);

    // xTaskCreate(uart1_event_task, "uart1_event_task", 2048, NULL, 12, &_hw_serial1.task_handle);

    return 0;
}
INIT_APP_EXPORT(hw_esp32_serial_init);

static void uart0_event_task(void *pvParameters)
{

    uart_event_t event;
    QueueHandle_t uart_queue = _hw_serial0.event_queue;
    uint8_t uart_port = _hw_serial0.usart_port;
    int32_t rx_size = _hw_serial0.config->rx_buffer_size;
    uint8_t *dtmp = (uint8_t *)malloc(rx_size);
    ESP_LOGI(TAG, "uart0_event_task create");
    for (;;) {
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, rx_size);
            ESP_LOGI(TAG, "uart0[%d] event", uart_port);
            switch (event.type) {
                case UART_DATA:
                    ESP_LOGI(TAG, "[UART0 DATA] : %d", event.size);
                    uart_read_bytes(uart_port, dtmp, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[DATA0 EVT]:");
                    uart_write_bytes(uart_port, (const char *)dtmp, event.size);
                    break;

                default:
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

static void uart1_event_task(void *pvParameters)
{
    uart_event_t event;
    QueueHandle_t uart_queue = _hw_serial1.event_queue;
    uint8_t uart_port = _hw_serial1.usart_port;
    int32_t rx_size = _hw_serial1.config->rx_buffer_size;
    uint8_t *dtmp = (uint8_t *)malloc(rx_size);
    for (;;) {
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, rx_size);
            ESP_LOGI(TAG, "uart1[%d] event", uart_port);
            switch (event.type) {
                case UART_DATA:
                    ESP_LOGI(TAG, "[UART1 DATA] : %d", event.size);
                    uart_read_bytes(uart_port, dtmp, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[DATA1 EVT]:");
                    uart_write_bytes(uart_port, (const char *)dtmp, event.size);
                    break;

                default:
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}
/*---------- end of file ----------*/
