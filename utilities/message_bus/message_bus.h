/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : message_bus.h
 * @Author       : Codex
 * @Date         : 2026-03-20 19:45:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 16:29:38
 * @Brief        : 轻量本地消息总线接口
 */

#ifndef __MESSAGE_BUS_H__
#define __MESSAGE_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "clists.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
enum msgbus_error {
    MSGBUS_ERR_NONE = 0,
    MSGBUS_ERR_INVALID_ARGS = 1,
    MSGBUS_ERR_NOT_FOUND = 2,
    MSGBUS_ERR_ALREADY_EXISTS = 3,
    MSGBUS_ERR_NO_MEMORY = 4,
    MSGBUS_ERR_NO_HANDLER = 5,
};

typedef struct message_bus *message_bus_t;
typedef struct msgbus_node *msgbus_node_t;
typedef struct msgbus_topic *msgbus_topic_t;
typedef struct msgbus_service *msgbus_service_t;

typedef void (*msgbus_topic_callback_t)(
    msgbus_node_t node, const char *topic, const void *msg, uint32_t size, void *user_data);
typedef int32_t (*msgbus_service_callback_t)(
    msgbus_node_t node, const void *req, uint32_t req_size, void *resp, uint32_t *resp_size, void *user_data);

struct message_bus {
    const char *name;
    struct list_head node_pool;
    struct list_head topic_pool;
    struct list_head service_pool;
};

struct msgbus_node {
    const char *name;
    message_bus_t bus;
    struct list_head node;
    struct list_head topics;
    struct list_head services;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化消息总线对象
 * @param  bus: 外部提供的消息总线存储空间
 * @param  name: 总线名称，仅用于标识
 */
void msgbus_init(message_bus_t bus, const char *name);

/**
 * @brief  反初始化消息总线，并清理其下所有节点资源
 * @param  bus: 目标消息总线
 */
void msgbus_deinit(message_bus_t bus);

/**
 * @brief  将节点注册到指定消息总线上
 * @param  node: 外部提供的节点存储空间
 * @param  bus: 目标消息总线
 * @param  name: 节点名称，要求在同一总线上唯一
 * @return true=成功, false=失败
 */
bool msgbus_node_init(msgbus_node_t node, message_bus_t bus, const char *name);

/**
 * @brief  注销节点，并释放该节点注册的 topic/service 项
 * @param  node: 目标节点
 */
void msgbus_node_deinit(msgbus_node_t node);

/**
 * @brief  按名称查找已注册节点
 * @param  bus: 目标消息总线
 * @param  name: 节点名称
 * @return 找到则返回节点指针，否则返回 NULL
 */
msgbus_node_t msgbus_node_find(message_bus_t bus, const char *name);

/**
 * @brief  按名称查找已注册 topic
 * @param  bus: 目标消息总线
 * @param  name: topic 名称
 * @return 找到则返回 topic 句柄，否则返回 NULL
 */
msgbus_topic_t msgbus_topic_find(message_bus_t bus, const char *name);

/**
 * @brief  按名称注册或获取 topic 句柄
 * @param  topic: 输出的 topic 句柄
 * @param  bus: 目标消息总线
 * @param  name: topic 名称
 * @return true=成功, false=失败
 */
bool msgbus_topic_register(msgbus_topic_t *topic, message_bus_t bus, const char *name);

/**
 * @brief  订阅指定 topic 句柄
 * @param  node: 订阅节点
 * @param  topic: topic 句柄
 * @param  cb: 收到消息后的回调
 * @param  user_data: 回调透传参数
 * @return true=成功, false=失败
 */
bool msgbus_topic_subscribe(msgbus_node_t node, msgbus_topic_t topic, msgbus_topic_callback_t cb, void *user_data);

/**
 * @brief  通过 topic 句柄发布消息
 * @param  topic: topic 句柄
 * @param  msg: 消息数据
 * @param  size: 消息字节数
 * @return 实际收到该消息的订阅者数量
 */
uint32_t msgbus_topic_publish(msgbus_topic_t topic, const void *msg, uint32_t size);

/**
 * @brief  注册服务处理函数并返回服务句柄
 * @param  service: 输出的服务句柄
 * @param  node: 提供服务的节点
 * @param  name: 服务名称，要求在同一总线上唯一
 * @param  cb: 服务处理回调
 * @param  user_data: 回调透传参数
 * @return true=成功, false=失败
 */
bool msgbus_service_advertise(
    msgbus_service_t *service, msgbus_node_t node, const char *name, msgbus_service_callback_t cb, void *user_data);

/**
 * @brief  按名称查找已注册服务
 * @param  bus: 目标消息总线
 * @param  name: 服务名称
 * @return 找到则返回服务句柄，否则返回 NULL
 */
msgbus_service_t msgbus_service_find(message_bus_t bus, const char *name);

/**
 * @brief  通过服务句柄调用服务
 * @param  service: 服务句柄
 * @param  req: 请求数据
 * @param  req_size: 请求字节数
 * @param  resp: 响应缓冲区，可为 NULL
 * @param  resp_size: 响应长度输入输出参数，可为 NULL
 * @return 0=成功, <0=失败
 */
int32_t msgbus_service_call(msgbus_service_t service, const void *req, uint32_t req_size, void *resp, uint32_t *resp_size);

#ifdef __cplusplus
}
#endif
#endif /* __MESSAGE_BUS_H__ */
