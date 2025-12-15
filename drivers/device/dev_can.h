/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : dev_can.h
 * @Author       : lxf
 * @Date         : 2025-12-11 14:26:10
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-15 14:05:57
 * @Brief        :
 */

#ifndef __DEV_CAN_H__
#define __DEV_CAN_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "define.h"
#include <stdint.h>
#include "device.h"
/*---------- macro ----------*/
#define IOCTL_CAN_SET_FILTER (IOCTL_USER_START + 0X00) // 设置过滤器
#define IOCTL_CAN_SET_BAUD   (IOCTL_USER_START + 0X01) // 设置波特率
#define IOCTL_CAN_SET_MODE   (IOCTL_USER_START + 0X02) // 设置工作模式
#define IOCTL_CAN_RXDONE     (IOCTL_USER_START + 0X03) // 接受完成（把rxMsg压入软件FIFO）
/*---------- type define ----------*/
/**
 * @brief 可配置的CAN波特率
 * @return {*}
 */
enum CANBAUD {
    CAN1MBaud = 1000UL * 1000,  /* 1 MBit/sec   */
    CAN800kBaud = 1000UL * 800, /* 800 kBit/sec */
    CAN500kBaud = 1000UL * 500, /* 500 kBit/sec */
    CAN250kBaud = 1000UL * 250, /* 250 kBit/sec */
    CAN125kBaud = 1000UL * 125, /* 125 kBit/sec */
    CAN100kBaud = 1000UL * 100, /* 100 kBit/sec */
    CAN50kBaud = 1000UL * 50,   /* 50 kBit/sec  */
    CAN20kBaud = 1000UL * 20,   /* 20 kBit/sec  */
    CAN10kBaud = 1000UL * 10    /* 10 kBit/sec  */
};

#define FP_CAN_MODE_NORMAL      0 // 正常模式
#define FP_CAN_MODE_LOOPBACK    1 // 回环模式
#define FP_CAN_MODE_SILENT      2 // 静默模式
#define FP_CAN_MODE_LISTENONLY  3 // 监听模式

#define FP_CAN_EVENT_RX_IND     0X01 // 接收中断
#define FP_CAN_EVENT_TX_DONE    0X02 // 发送完成
#define FP_CAN_EVENT_TX_FAIL    0x03 // 发送失败
#define FP_CAN_EVENT_RXOF_IND   0x04 // 接收overflow
#define FP_CAN_EVENT_TX_TIMEOUT 0x05 // 接收超时

struct dev_can_describe; // 前向声明

/**
 * @brief 为CAN驱动的基本配置信息
 * @return {*}
 */
struct fp_can_configure {
    enum CANBAUD baud; // CAN波特率
    uint8_t msgboxsz;  // CAN接收邮箱缓冲数量（软件层面）
    uint8_t mode;      // CAN工作模式
};

/**
 * @brief CAN消息结构体
 * @return {*}
 */
struct can_msg {
    uint32_t id;     // CANID
    uint32_t ide;    // 0:标准帧，1：扩展帧
    uint32_t rtr;    // 0:数据帧，1：远程帧
    uint8_t dlc;     // 数据载荷长度
    uint8_t data[8]; // 数据载荷
    uint32_t hdr;    // 硬件过滤索引（接收时使用）
};

struct can_msg_list {
    struct can_msg *msg;
    uint8_t read;
    uint8_t write;
    uint8_t count;
    uint8_t max;
};

/**
 * @brief 底层驱动操作接口（OPS）
 * @return {*}
 */
struct fp_can_ops {
    /**
     * @brief 配置波特率、模式等
     * @param {dev_can_describe} *self
     * @param {fp_can_configure} *cfg
     * @return {*}
     */
    fp_err_t (*init)(struct dev_can_describe *self, struct fp_can_configure *cfg);
    /**
     * @brief 控制命令（开关中断、设置过滤器）
     * @param {dev_can_describe} *self
     * @param {int} cmd
     * @param {void} *args
     * @return {*}
     */
    fp_err_t (*control)(struct dev_can_describe *self, int cmd, void *args);
    /**
     * @brief 发送一帧数据
     * @param {dev_can_describe} *self
     * @param {void} *buf
     * @param {int} box_num
     * @return {*}
     */
    fp_err_t (*sendmsg)(struct dev_can_describe *self, struct can_msg *msg);
};

struct dev_can_describe {
    struct fp_can_ops *ops;         /* 底层操作接口 */
    struct fp_can_configure config; /* 当前配置 */

    /* 接收FIFO（软件缓冲） */
    struct can_msg_list rx_msg; /* 软件rx缓冲 */
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
