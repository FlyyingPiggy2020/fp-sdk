 /*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : paj7620.c
 * @Author       : lxf
 * @Date         : 2024-12-10 11:43:23
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-11 14:54:17
 * @Brief        :
 */

/*---------- includes ----------*/
#include "paj7620.h"
#include "drv_err.h"
#include "driver.h"
#include <stdbool.h>
#include "i2c_bus.h"
/*---------- macro ----------*/
#define PAJ_REGITER_BANK_SEL      0XEF // BANK选择寄存器
#define PAJ_BANK0                 0X00 // BANK0
#define PAJ_BANK1                 0X01 // BANK1

// BANK0 寄存器组
#define PAJ_SUSPEND_CMD           0X03 // 设置设备挂起
#define PAJ_SET_INT_FLAG1         0X41 // 设置手势检测中断寄存器1
#define PAJ_SET_INT_FLAG2         0X42 // 设置手势检测中断寄存器2
#define PAJ_GET_INT_FLAG1         0X43 // 获取手势检测中断标志寄存器1(获取手势结果)
#define PAJ_GET_INT_FLAG2         0X44 // 获取手势检测中断标志寄存器2(获取手势结果)
#define PAJ_GET_STATE             0X45 // 获取手势检测工作状态
#define PAJ_SET_HIGH_THRESHOLD    0x69 // 设置滞后高阀值（仅在接近检测模式下）
#define PAJ_SET_LOW_THRESEHOLD    0X6A // 设置滞后低阀值
#define PAJ_GET_APPROACH_STATE    0X6B // 获取接近状态 （1：PS data>= PS threshold ,0:PS data<= Low threshold）
#define PAJ_GET_GESTURE_DATA      0X6C // 获取接近数据
#define PAJ_GET_OBJECT_BRIGHTNESS 0XB0 // 获取被照物体亮度（最大255）
#define PAJ_GET_OBJECT_SIZE_1     0XB1 // 获取被照物体大小低八位（bit7:0）(最大900)
#define PAJ_GET_OBJECT_SIZE_2     0XB2 // 获取被照物体大小高四位（bit3:0）

// BANK1 寄存器组
#define PAJ_SET_PS_GAIN           0X44 // 设置检测增益大小 (0:1x gain 1:2x gain)
#define PAJ_SET_IDLE_S1_STEP_0    0x67 // 设置S1的响应因子
#define PAJ_SET_IDLE_S1_STEP_1    0x68
#define PAJ_SET_IDLE_S2_STEP_0    0X69 // 设置S2的响应因子
#define PAJ_SET_IDLE_S2_STEP_1    0X6A
#define PAJ_SET_OP_TO_S1_STEP_0   0X6B // 设置OP到S1的过度时间
#define PAJ_SET_OP_TO_S1_STEP_1   0X6C
#define PAJ_SET_S1_TO_S2_STEP_0   0X6D // 设置S1到S2的过度时间
#define PAJ_SET_S1_TO_S2_STEP_1   0X6E
#define PAJ_OPERATION_ENABLE      0X72 // 设置PAJ7620U2使能寄存器

// 手势识别效果
#define BIT(x)                    1 << (x)

#define GES_UP                    BIT(0) // 向上
#define GES_DOWN                  BIT(1) // 向下
#define GES_LEFT                  BIT(2) // 向左
#define GES_RIGHT                 BIT(3) // 向右
#define GES_FORWARD               BIT(4) // 向前
#define GES_BACKWARD              BIT(5) // 向后
#define GES_CLOCKWISE             BIT(6) // 顺时针
#define GES_COUNT_CLOCKWISE       BIT(7) // 逆时针
#define GES_WAVE                  BIT(8) // 挥动

/*---------- type define ----------*/

typedef enum {
    BANK0 = 0,
    BANK1,
} bank_e;

/*---------- variable prototype ----------*/
const uint8_t paj7620_init_regs[][2] = {
    { 0xEF, 0x00 },
    { 0x41, 0xFF }, // R_Int_1_En[7:0]
    { 0x42, 0x01 }, // R_Int_2_En[7:0]
    { 0x46, 0x2D }, // R_AELedOff_UB[7:0]
    { 0x47, 0x0F }, // R_AELedOff_LB[7:0]
    { 0x48, 0xE0 }, // R_AE_Exposure_UB[7:0]
    { 0x49, 0x00 }, // R_AE_Exposure_UB[15:8]
    { 0x4A, 0x70 }, // R_AE_Exposure_LB[7:0]
    { 0x4B, 0x00 }, // R_AE_Exposure_LB[15:8]
    { 0x4C, 0x20 }, // R_AE_Gain_UB[7:0]
    { 0x4D, 0x00 }, // R_AE_Gain_LB[7:0]
    { 0x51, 0x10 }, // R_AE_EnH = 1
    { 0x5C, 0x02 }, // R_SenClkPrd[5:0]
    { 0x5E, 0x10 }, // R_SRAM_CLK_manual
    { 0x80, 0x42 }, // GPIO setting
    { 0x81, 0x44 }, // GPIO setting
    { 0x82, 0x0C }, // Interrupt IO setting
    { 0x83, 0x20 }, // R_LightThd[7:0]
    { 0x84, 0x20 }, // R_ObjectSizeStartTh[7:0]
    { 0x85, 0x00 }, // R_ObjectSizeStartTh[9:8]
    { 0x86, 0x10 }, // R_ObjectSizeEndTh[7:0]
    { 0x87, 0x00 }, // R_ObjectSizeEndTh[9:8]
    { 0x8B, 0x01 }, // R_Cursor_ObjectSizeTh[7:0]
    { 0x8D, 0x00 }, // R_TimeDelayNum[7:0]
    { 0x90, 0x0C }, // R_NoMotionCountThd[6:0]
    { 0x91, 0x0C }, // R_NoObjectCountThd[6:0]
    { 0x93, 0x0D }, // R_XDirectionThd[4:0]
    { 0x94, 0x0A }, // R_YDirectionThd[4:0]
    { 0x95, 0x0A }, // R_ZDirectionThd[4:0]
    { 0x96, 0x0C }, // R_ZDirectionXYThd[4:0]
    { 0x97, 0x05 }, // R_ZDirectionAngleThd[3:0]
    { 0x9A, 0x14 }, // R_RotateXYThd[4:0]
    { 0x9C, 0x3F }, // Filter setting
    { 0x9F, 0xF9 }, // R_UseBGModel is enable
    { 0xA0, 0x48 }, // R_BGUpdateMaxIntensity[7:0]
    { 0xA5, 0x19 }, // R_FilterAverage_Mode
    { 0xAA, 0x14 },
    { 0xAB, 0x14 },
    { 0xCC, 0x19 }, // R_YtoZSum[5:0]
    { 0xCD, 0x0B }, // R_YtoZFactor[5:0]
    { 0xCE, 0x13 }, // bit[2:0] = R_PositionFilterLength[2:0]
    { 0xCF, 0x63 }, // bit[3:0] = R_WaveCountThd[3:0]
    { 0xD0, 0x21 }, // R_AbortXYRatio[4:0] & R_AbortLength[6:0]
    { 0xEF, 0x01 },
    { 0x00, 0x14 }, // Cmd_HSize[5:0]
    { 0x01, 0x14 }, // Cmd_VSize[5:0]
    { 0x02, 0x14 }, // Cmd_HStart[5:0]
    { 0x03, 0x14 }, // Cmd_VStart[5:0]
    { 0x04, 0x02 }, // Sensor skip & flip
    { 0x25, 0x00 },
    { 0x26, 0x00 }, // R_OffsetX[6:0]
    { 0x27, 0x39 }, // R_OffsetY[6:0]
    { 0x28, 0x7F }, // R_LSC[6:0]
    { 0x29, 0x08 }, // R_LSFT[3:0]
    { 0x30, 0x03 }, // R_LED_SoftStart_time[7:0]
    { 0x3E, 0xFF }, // Cmd_DebugPattern[7:0]
    { 0x4A, 0x14 },
    { 0x4B, 0x14 },
    { 0x4C, 0x00 },
    { 0x4D, 0x00 },
    { 0x5E, 0x3D }, // analog voltage setting
    { 0x65, 0xAC }, //  R_IDLE_TIME[7:0]
    { 0x66, 0x00 }, // R_IDLE_TIME[15:8]
    { 0x67, 0x97 }, // R_IDLE_TIME_SLEEP_1[7:0]
    { 0x68, 0x01 }, // R_IDLE_TIME_SLEEP_1[15:8]
    { 0x69, 0xCD }, // R_IDLE_TIME_SLEEP_2[7:0]
    { 0x6A, 0x01 }, // R_IDLE_TIME_SLEEP_2[15:8]
    { 0x6B, 0xB0 }, // R_Obj_TIME_1[7:0]
    { 0x6C, 0x04 }, // R_Obj_TIME_1[15:8]
    { 0x6D, 0x2C }, // R_Obj_TIME_2[7:0]
    { 0x6E, 0x01 }, // R_Obj_TIME_2[15:8]
    { 0x72, 0x01 }, // R_TG_EnH(使能寄存器,0:Disable,1:Enable)
    { 0x73, 0x35 }, // Auto Sleep & Wakeup mode
    { 0x74, 0x00 }, // R_Control_Mode[2:0]
    { 0x77, 0x01 }, // R_SRAM_Read_EnH
    { 0x7C, 0x90 },
    { 0x7D, 0x01 }
};
/*---------- function prototype ----------*/
static int32_t paj7620_open(driver_t **pdrv);
static void paj7620_close(driver_t **pdrv);
static int32_t paj7620_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t paj7620_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);
///* ioctl callback function */
static int32_t _ioctl_set_evt_cb(paj7620_describe_t *pdesc, void *args);
// low level function
static int32_t paj7620u2_weakup(paj7620_describe_t *pdesc);
static uint8_t _gs_write_reg(paj7620_describe_t *pdesc, uint8_t reg_address, uint8_t reg_data);
static uint8_t _gs_read_bytes(paj7620_describe_t *pdesc, uint8_t reg_address);
static void _paj7620_select_bank(paj7620_describe_t *pdesc, bank_e bank);

/*---------- variable ----------*/
DRIVER_DEFINED(paj7620, paj7620_open, paj7620_close, NULL, NULL, paj7620_ioctl, paj7620_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_PAJ7620_SET_EVT_CB, _ioctl_set_evt_cb },

};
/*---------- function ----------*/

static int32_t paj7620_open(driver_t **pdrv)
{
    paj7620_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    void *bus = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    uint8_t count;
    ASSERT(pdrv != NULL);

    do {
        if (!pdesc) {
            break;
        }
        err = DRV_ERR_OK;
        /* 绑定i2c总线 */
        if (NULL == (bus = device_open(pdesc->bus_name))) {
            TRACE("%s bind %s failed!\n", container_of(pdrv, device_t, pdrv)->dev_name, pdesc->bus_name);
            if (pdesc->ops.deinit) {
                pdesc->ops.deinit();
            }
            err = DRV_ERR_ERROR;
            break;
        }
        pdesc->bus = bus;
        // 唤醒PAJ7620
        count = 5;
        while (count) {
            count--;
            err = paj7620u2_weakup(pdesc);
            if (err == DRV_ERR_OK) {
                for (uint16_t i = 0; i < sizeof(paj7620_init_regs) / 2; i++) {
                    _gs_write_reg(pdesc, paj7620_init_regs[i][0], paj7620_init_regs[i][1]);
                }
                // 初始化中断
                if (pdesc->ops.init) {
                    if (!pdesc->ops.init()) {
                        err = DRV_ERR_ERROR;
                    }
                }
                break;
            } else {
                pdesc->ops.delay_ms(10);
            }
        }
    } while (0);
    return err;
}
static void paj7620_close(driver_t **pdrv)
{
    paj7620_describe_t *pdesc = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc) {
        if (pdesc->bus) {
            device_close(pdesc->bus);
            pdesc->bus = NULL;
        }
        if (pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
    }
    return;
}
static int32_t paj7620_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    paj7620_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(paj7620_describe_t *, void *) = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t(*)(paj7620_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}

static paj7620_evt_t _status_to_evt(paj7620_describe_t *pdesc, uint16_t status)
{
    paj7620_evt_t evt = PAJ7620_EVT_NONE;
#define DIR(x) ((x + pdesc->config.dir) % 4)
    paj7620_evt_t table[4] = { PAJ7620_EVT_UP, PAJ7620_EVT_RIGHT, PAJ7620_EVT_DOWN, PAJ7620_EVT_LEFT };
    uint8_t test = 0;
    switch (status) {
        case GES_UP:
            evt = table[DIR(0)];
            break;
        case GES_RIGHT:
            evt = table[DIR(1)];
            break;
        case GES_DOWN:
            evt = table[DIR(2)];
            ;
            break;
        case GES_LEFT:
            test = DIR(3);
            evt = table[DIR(3)];
            ;
            break;
        case GES_FORWARD:
            evt = PAJ7620_EVT_FORWARD;
            break;
        case GES_BACKWARD:
            evt = PAJ7620_EVT_BACKWARD;
            break;
        case GES_CLOCKWISE:
            evt = PAJ7620_EVT_CW;
            break;
        case GES_COUNT_CLOCKWISE:
            evt = PAJ7620_EVT_CCW;
            break;
        case GES_WAVE:
            evt = PAJ7620_EVT_WAVE;
            break;
        default:
            break;
    }
    return evt;
}
static int32_t paj7620_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    paj7620_describe_t *pdesc = NULL;
    uint16_t status;
    paj7620_evt_t evt;
    ASSERT(pdrv);
    ASSERT(args);

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc) {
        // 1. check bank0
        _paj7620_select_bank(pdesc, BANK0);
        status = _gs_read_bytes(pdesc, PAJ_GET_INT_FLAG2);
        status <<= 8;
        status += _gs_read_bytes(pdesc, PAJ_GET_INT_FLAG1);
        if (status) {
            evt = _status_to_evt(pdesc, status);
            if (pdesc->cb.event) {
                pdesc->cb.event(evt);
                return DRV_ERR_OK;
            }
        } else {
            return DRV_ERR_ERROR;
        }
    }
    return DRV_ERR_POINT_NONE;
}

///* ioctl callback function */
static void __evt_cb_default(paj7620_evt_t evt)
{
}

static int32_t _ioctl_set_evt_cb(paj7620_describe_t *pdesc, void *args)
{
    void (*cb)(paj7620_evt_t) = (void (*)(paj7620_evt_t))args;
    pdesc->cb.event = (cb ? cb : __evt_cb_default);
    return DRV_ERR_OK;
}

///* low level function */
static uint8_t _gs_write_reg(paj7620_describe_t *pdesc, uint8_t reg_address, uint8_t reg_data)
{
    i2c_msg_t msgs[2];

    uint8_t buf[2] = { reg_address, reg_data };
    msgs[0].addr = pdesc->config.ee_dev_addr;
    msgs[0].flags = I2C_BUS_WR;
    msgs[0].buf = buf;
    msgs[0].len = 2;

    i2c_priv_data_t arg = {
        .msgs = (i2c_msg_t *)&msgs,
        .number = 1,
    };

    device_ioctl(pdesc->bus, IOCTL_I2CBUS_CTRL_RW, &arg);
    return reg_data;
}

static uint8_t _gs_read_bytes(paj7620_describe_t *pdesc, uint8_t reg_address)
{
    i2c_msg_t msgs[2];
    uint8_t data;
    msgs[0].addr = pdesc->config.ee_dev_addr;
    msgs[0].flags = I2C_BUS_WR;
    msgs[0].buf = &reg_address;
    msgs[0].len = 1;

    msgs[1].addr = pdesc->config.ee_dev_addr;
    msgs[1].flags = I2C_BUS_RD;
    msgs[1].buf = &data;
    msgs[1].len = 1;

    i2c_priv_data_t arg = {
        .msgs = (i2c_msg_t *)&msgs,
        .number = 2,
    };
    device_ioctl(pdesc->bus, IOCTL_I2CBUS_CTRL_RW, &arg);
    return data;
}

static uint8_t _gs_weakup(paj7620_describe_t *pdesc)
{
    i2c_msg_t msgs[2];
    uint8_t txbuf[2] = { PAJ_REGITER_BANK_SEL, PAJ_BANK0 };
    msgs[0].addr = pdesc->config.ee_dev_addr;
    msgs[0].flags = I2C_BUS_WR;
    msgs[0].buf = txbuf;
    msgs[0].len = 2;

    i2c_priv_data_t arg = {
        .msgs = (i2c_msg_t *)&msgs,
        .number = 1,
    };

    return device_ioctl(pdesc->bus, IOCTL_I2CBUS_CTRL_RW, &arg);
}

static void _paj7620_select_bank(paj7620_describe_t *pdesc, bank_e bank)
{
    switch (bank) {
        case BANK0:
            _gs_write_reg(pdesc, PAJ_REGITER_BANK_SEL, PAJ_BANK0);
            break;
        case BANK1:
            _gs_write_reg(pdesc, PAJ_REGITER_BANK_SEL, PAJ_BANK1);
            break;
        default:
            break;
    }
}

static int32_t paj7620u2_weakup(paj7620_describe_t *pdesc)
{
    uint16_t data;
    _gs_weakup(pdesc);
    pdesc->ops.delay_ms(10);

    _paj7620_select_bank(pdesc, BANK0); // 进入BANK0
    data = _gs_read_bytes(pdesc, 0x01); // 读取状态
    data <<= 8;
    data += _gs_read_bytes(pdesc, 0x00); // 读取状态
    if (data != 0x7620) {
        return DRV_ERR_ERROR;
    }
    return DRV_ERR_OK;
}
/*---------- end of file ----------*/
