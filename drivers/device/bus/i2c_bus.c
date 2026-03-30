/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : components/fp-sdk/drivers/device/bus/i2c_bus.c
 * @Author       : lxf
 * @Date         : 2024-12-08 11:54:33
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-05 13:28:49
 * @brief        : I2C总线驱动(位模拟方式)
 * @features     :
 *               - GPIO位模拟I2C协议(SCL/SDA)
 *               - 支持7位/10位设备地址
 *               - 支持多消息连续传输(RESTART条件)
 *               - 支持重试机制(可配置重试次数)
 *               - 总线恢复功能(SDA卡死自动恢复)
 *               - 可选的锁保护(多线程安全)
 *               - 灵活的标志位控制(启停/ACK/NACK等)
 *
 * @usage        :
 *               @code
 *               // 1. 定义I2C设备(在bsp层实现)
 *               static bool my_i2c_init(void) {
 *                   // 配置GPIO为开漏输出
 *                   return true;
 *               }
 *               static i2c_bit_ops_t ops = {
 *                   .init = my_i2c_init,
 *                   .scl_set = my_scl_set,
 *                   .scl_get = my_scl_get,
 *                   .sda_set = my_sda_set,
 *                   .sda_get = my_sda_get,
 *                   .udelay = my_udelay,
 *                   .delay_us = 5,  // 5us延时,适应100kHz频率
 *               };
 *               static i2cbus_describe_t i2cbus = {
 *                   .ops = &ops,
 *                   .retries = 1000,
 *               };
 *               DEVICE_DEFINED(my_i2c, i2c_bus, &i2cbus);
 *
 *               // 2. 使用I2C设备
 *               device_t *i2c = device_open("my_i2c");
 *
 *               // 3. 写入数据
 *               uint8_t tx_buf[2] = {0x10, 0xAA};  // 寄存器地址 + 数据
 *               i2c_msg_t msg = {
 *                   .addr = 0x50 << 1,      // 7位地址左移1位
 *                   .flags = I2C_BUS_WR,    // 写操作
 *                   .len = 2,
 *                   .buf = tx_buf,
 *               };
 *               i2c_priv_data_t priv = {
 *                   .msgs = &msg,
 *                   .number = 1,
 *               };
 *               device_ioctl(i2c, IOCTL_I2CBUS_CTRL_RW, &priv);
 *
 *               // 4. 读取数据(先写寄存器地址,再读数据)
 *               uint8_t reg_addr = 0x10;
 *               uint8_t rx_data;
 *               i2c_msg_t msgs[2] = {
 *                   {.addr = 0x50 << 1, .flags = I2C_BUS_WR, .len = 1, .buf = &reg_addr},
 *                   {.addr = 0x50 << 1, .flags = I2C_BUS_RD, .len = 1, .buf = &rx_data},
 *               };
 *               i2c_priv_data_t priv = {.msgs = msgs, .number = 2};
 *               device_ioctl(i2c, IOCTL_I2CBUS_CTRL_RW, &priv);
 *               @endcode
 *
 * @note         位模拟方式通过GPIO控制SCL/SDA实现I2C协议,无需硬件I2C外设。
 *               适用于任意具备GPIO的MCU,便于移植和调试。
 *
 * @warning      GPIO必须配置为开漏输出模式(Open-Drain),并使用上拉电阻。
 *               delay_us需根据实际时钟频率调整,典型值: 100kHz->5us, 400kHz->1us。
 */

/*---------- includes ----------*/
#include "options.h"
#include "i2c_bus.h"
#include "driver.h"
/*---------- macro ----------*/
#define SET_SDA(ops, val) ops->sda_set(val) /* 设置SDA电平 */
#define SET_SCL(ops, val) ops->scl_set(val) /* 设置SCL电平 */
#define GET_SDA(ops)      ops->sda_get()    /* 读取SDA电平 */
#define GET_SCL(ops)      ops->scl_get()    /* 读取SCL电平 */

#define SDA_L(ops)        SET_SDA(ops, false) /* SDA拉低 */
#define SDA_H(ops)        SET_SDA(ops, true)  /* SDA拉高 */
#define SCL_L(ops)        SET_SCL(ops, false) /* SCL拉低 */
#define SCL_H(ops)        SET_SCL(ops, true)  /* SCL拉高 */
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t i2c_open(driver_t **pdrv);
static void i2c_close(driver_t **pdrv);
static int32_t i2c_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

static int32_t i2c_bus_device_control(i2cbus_describe_t *bus, void *arg);
static int32_t i2c_transfer(i2cbus_describe_t *bus, i2c_msg_t msgs[], uint32_t num);

//
static void i2c_stop(i2c_bit_ops_t *ops);
/*---------- variable ----------*/
DRIVER_DEFINED(i2c_bus, i2c_open, i2c_close, NULL, NULL, i2c_ioctl, NULL);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_I2CBUS_CTRL_RW, i2c_bus_device_control },
};
/*---------- function ----------*/
/**
 * @brief  打开I2C总线设备
 * @param  pdrv: 驱动指针
 * @return 0=成功, <0=失败
 */
static int32_t i2c_open(driver_t **pdrv)
{
    i2cbus_describe_t *pdesc = NULL;
    int32_t err = E_WRONG_ARGS;

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    assert(pdrv != NULL);

    do {
        if (!pdesc) {
            break;
        }
        err = E_OK;
        if (pdesc->ops->init) {
            if (!pdesc->ops->init()) {
                err = E_ERROR;
                break;
            }
            i2c_stop(pdesc->ops);
        }
    } while (0);
    return err;
}

/**
 * @brief  关闭I2C总线设备
 * @param  pdrv: 驱动指针
 * @return 无
 */
static void i2c_close(driver_t **pdrv)
{
    i2cbus_describe_t *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    assert(pdesc != NULL);
    if (pdesc->ops->deinit) {
        pdesc->ops->deinit();
    }
}

/**
 * @brief  I2C总线IO控制
 * @param  pdrv: 驱动指针
 * @param  cmd: 控制命令
 * @param  args: 参数指针
 * @return 0=成功, <0=失败
 */
static int32_t i2c_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    int32_t err = E_WRONG_ARGS;
    i2cbus_describe_t *pdesc = NULL;
    int32_t (*cb)(i2cbus_describe_t *, void *) = NULL;
    assert(pdrv);

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t (*)(i2cbus_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}

/**
 * @brief  I2C延时
 * @param  ops: 位操作接口
 * @return 无
 */
static inline void i2c_delay(i2c_bit_ops_t *ops)
{
    ops->udelay(ops->delay_us);
}

/**
 * @brief  I2C总线恢复(当SDA被拉低时)
 * @param  ops: 位操作接口
 * @return 无
 */
static void i2c_recover(i2c_bit_ops_t *ops)
{
    for (uint8_t i = 0; i < 32; i++) {
        SCL_L(ops);
        i2c_delay(ops);
        SCL_H(ops);
        i2c_delay(ops);
        if (GET_SDA(ops)) {
            break;
        }
    }
}

/**
 * @brief  I2C重启信号
 * @param  ops: 位操作接口
 * @return 无
 */
static void i2c_restart(i2c_bit_ops_t *ops)
{
    SDA_H(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

/**
 * @brief  I2C起始信号
 * @param  ops: 位操作接口
 * @return 无
 */
static void i2c_start(i2c_bit_ops_t *ops)
{
    if (!GET_SDA(ops)) {
        i2c_recover(ops);
    }
    SDA_H(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
    i2c_delay(ops);
}

/**
 * @brief  I2C停止信号
 * @param  ops: 位操作接口
 * @return 无
 */
static void i2c_stop(i2c_bit_ops_t *ops)
{
    SDA_L(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_H(ops);
    i2c_delay(ops);
}

/**
 * @brief  等待I2C从机应答
 * @param  ops: 位操作接口
 * @return 0=收到ACK, <0=无应答(NACK)
 */
static int32_t i2c_waitack(i2c_bit_ops_t *ops)
{
    int32_t re;
    SDA_H(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    if (GET_SDA(ops)) {
        re = E_ERROR;
    } else {
        re = E_OK;
    }
    SCL_L(ops);
    i2c_delay(ops);
    return re;
}

/**
 * @brief  写入一个字节
 * @param  bus: I2C总线描述符
 * @param  data: 要写入的数据
 * @return 0
 */
static uint32_t i2c_writeb(i2cbus_describe_t *bus, uint8_t data)
{
    i2c_bit_ops_t *ops = bus->ops;

    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x80) {
            SDA_H(ops);
        } else {
            SDA_L(ops);
        }
        i2c_delay(ops);
        SCL_H(ops);
        i2c_delay(ops);
        SCL_L(ops);
        if (i == 7) {
            SDA_H(ops);
        }
        data <<= 1;
        i2c_delay(ops);
    }
    return 0;
}

/**
 * @brief  读取一个字节
 * @param  bus: I2C总线描述符
 * @return 读取到的数据
 */
static uint8_t i2c_readb(i2cbus_describe_t *bus)
{
    uint8_t i;
    uint8_t value;
    i2c_bit_ops_t *ops = bus->ops;
    value = 0;
    for (i = 0; i < 8; i++) {
        value <<= 1;
        SCL_H(ops);
        i2c_delay(ops);
        if (GET_SDA(ops)) {
            value++;
        }
        SCL_L(ops);
        i2c_delay(ops);
    }
    return value;
}

/**
 * @brief  发送多字节数据
 * @param  bus: I2C总线描述符
 * @param  msg: I2C消息结构
 * @return 实际发送字节数, <0=失败(没收到应答)
 */
static int32_t i2c_send_bytes(i2cbus_describe_t *bus, i2c_msg_t *msg)
{
    int32_t bytes = 0;
    const uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    i2c_bit_ops_t *ops = bus->ops;

    while (count > 0) {
        i2c_writeb(bus, *ptr);
        if (i2c_waitack(ops) != E_OK) {
            return E_TIME_OUT;
        }
        count--;
        ptr++;
        bytes++;
    }
    return bytes;
}

/**
 * @brief  发送ACK应答
 * @param  bus: I2C总线描述符
 * @return 0
 */
static int32_t i2c_ack(i2cbus_describe_t *bus)
{
    i2c_bit_ops_t *ops = bus->ops;

    SDA_L(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SCL_L(ops);
    i2c_delay(ops);
    SDA_H(ops);
    return 0;
}

/**
 * @brief  发送NACK非应答
 * @param  bus: I2C总线描述符
 * @return 0
 */
static int32_t i2c_nack(i2cbus_describe_t *bus)
{
    i2c_bit_ops_t *ops = bus->ops;

    SDA_H(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SCL_L(ops);
    i2c_delay(ops);
    return 0;
}

/**
 * @brief  接收多字节数据
 * @param  bus: I2C总线描述符
 * @param  msg: I2C消息结构
 * @return 实际接收字节数
 */
static int32_t i2c_recv_bytes(i2cbus_describe_t *bus, i2c_msg_t *msg)
{
    int32_t val;
    int32_t bytes = 0; /* actual byetes */
    uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    uint32_t flags = msg->flags;

    while (count > 0) {
        val = i2c_readb(bus);
        *ptr = val;
        bytes++;
        ptr++;
        count--;
        if (!(flags & I2C_NO_READ_ACK)) {
            if (count) {
                i2c_ack(bus);
            } else {
                i2c_nack(bus);
            }
        }
    }
    return bytes;
}

/**
 * @brief  发送I2C设备地址(带重试)
 * @param  bus: I2C总线描述符
 * @param  addr: 设备地址
 * @param  retries: 重试次数
 * @return 0=成功, <0=失败
 */
static int32_t i2c_send_address(i2cbus_describe_t *bus, uint8_t addr, int32_t retries)
{
    i2c_bit_ops_t *ops = bus->ops;

    int32_t i;
    int32_t ret = E_TIME_OUT;

    for (i = 0; i <= retries; i++) {
        i2c_writeb(bus, addr);
        ret = i2c_waitack(ops);
        if (ret == E_OK || i == retries) {
            break;
        }
        i2c_stop(ops);
        i2c_delay(ops);
        i2c_start(ops);
    }
    return ret;
}

/**
 * @brief  发送I2C设备地址(支持7位/10位地址)
 * @param  bus: I2C总线描述符
 * @param  msg: I2C消息结构
 * @return 0=成功, <0=失败
 */
static int32_t i2c_bit_send_address(i2cbus_describe_t *bus, i2c_msg_t *msg)
{
    uint16_t flags = msg->flags;
    uint16_t ignore_nack = msg->flags & I2C_IGNORE_NACK;
    i2c_bit_ops_t *ops = bus->ops;

    uint8_t addr1, addr2;
    int32_t retries;
    int32_t ret;

    retries = ignore_nack ? 0 : bus->retries;

    if (flags & I2C_ADDR_10BIT) {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0Xff;
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != E_OK) && !ignore_nack) {
            return ret;
        }

        i2c_writeb(bus, addr2);
        ret = i2c_waitack(ops);
        if (ret != E_OK && !ignore_nack) {
            return ret;
        }
        if (flags & I2C_BUS_RD) {
            i2c_restart(ops);
            addr1 |= 0x01;
            ret = i2c_send_address(bus, addr1, retries);
            if ((ret != E_OK) && !ignore_nack) {
                return ret;
            }
        }
    } else {
        /* 7 bit addr */
        addr1 = msg->addr << 1;
        if (flags & I2C_BUS_RD) {
            addr1 |= 1;
        }
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != E_OK) && !ignore_nack) {
            return ret;
        }
    }
    return E_OK;
}

/**
 * @brief  I2C位模拟传输(多消息)
 * @param  bus: I2C总线描述符
 * @param  msgs: I2C消息数组
 * @param  num: 消息数量
 * @return 实际处理的消息数, <0=失败
 */
static int32_t i2c_bit_xfer(i2cbus_describe_t *bus, i2c_msg_t msgs[], uint32_t num)
{
    i2c_msg_t *msg;
    i2c_bit_ops_t *ops = bus->ops;
    int32_t ret = 0;
    uint32_t i;
    uint16_t ignore_nack;

    if (num == 0) {
        return 0;
    }

    for (i = 0; i < num; i++) {
        msg = &msgs[i];
        ignore_nack = msg->flags & I2C_IGNORE_NACK;
        if (!(msg->flags & I2C_NO_START)) {
            if (i) {
                i2c_restart(ops);
            } else {
                i2c_start(ops);
            }
            ret = i2c_bit_send_address(bus, msg);
            if (ret != E_OK && !ignore_nack) {
                goto out;
            }
        }
        if (msg->flags & I2C_BUS_RD) {
            i2c_recv_bytes(bus, msg);
            ret = E_OK;
        } else {
            ret = i2c_send_bytes(bus, msg);
            if (ret < msg->len) {
                if (ret >= 0) {
                    ret = E_WRONG_ARGS;
                }
                goto out;
            }
        }
    }
    ret = i;
out:
    if (!(msg->flags & I2C_NO_STOP)) {
        i2c_stop(ops);
    }
    return ret;
}

/**
 * @brief  I2C数据传输(带锁保护)
 * @param  bus: I2C总线描述符
 * @param  msgs: I2C消息数组
 * @param  num: 消息数量
 * @return 实际处理的消息数, <0=失败
 */
static int32_t i2c_transfer(i2cbus_describe_t *bus, i2c_msg_t msgs[], uint32_t num)
{
    int32_t ret;

    i2c_bit_ops_t *ops = bus->ops;

    if (ops->lock) {
        ops->lock(ops);
    }
    ret = i2c_bit_xfer(bus, msgs, num);
    if (ops->unlock) {
        ops->unlock(ops);
    }
    return ret;
}

/**
 * @brief  I2C总线设备控制
 * @param  bus: I2C总线描述符
 * @param  arg: 私有数据指针(i2c_priv_data_t)
 * @return 0=成功, <0=失败
 */
static int32_t i2c_bus_device_control(i2cbus_describe_t *bus, void *arg)
{
    i2c_priv_data_t *priv_data;
    int32_t ret;
    assert(bus);
    assert(bus->ops);

    priv_data = arg;
    ret = i2c_transfer(bus, priv_data->msgs, priv_data->number);
    return (ret == priv_data->number) ? E_OK : E_ERROR;
}
/*---------- end of file ----------*/
