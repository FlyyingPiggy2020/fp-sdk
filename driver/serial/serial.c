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
 * @FilePath     : serial.c
 * @Author       : lxf
 * @Date         : 2024-03-16 13:22:36
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-16 13:24:11
 * @Brief        : serial is a full-dup communication method
 */

/*---------- includes ----------*/

#include "serial.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

int device_serial_register(struct device_serial *serial, const char *name, struct serial_ops *ops)
{
    serial->parent.rx_indicate = NULL;
    serial->parent.tx_complete = NULL;
    serial->parent.ops = NULL;
    serial->ops = ops;

    device_register(&serial->parent, name, DEVICE_FLAG_RDWR);
    return 0;
}

fp_size_t serial_read(struct device *device, uint8_t *buff, fp_size_t wanted_size)
{
    struct device_serial *serial = (struct device_serial *)device;

    assert(serial->ops != NULL);

    return serial->ops->serial_read(device, buff, wanted_size);
}

void serial_write(struct device *device, uint8_t *buff, fp_size_t size)
{
    struct device_serial *serial = (struct device_serial *)device;

    assert(serial->ops != NULL);

    return serial->ops->serial_write(device, buff, size);
}

fp_size_t serial_config(struct device *device, struct serial_config *config, void (*idle_callback)(fp_size_t event_size))
{
    struct device_serial *serial = (struct device_serial *)device;

    assert(serial->ops != NULL);

    return serial->ops->serial_config(device, config, idle_callback);
}
/*---------- end of file ----------*/
