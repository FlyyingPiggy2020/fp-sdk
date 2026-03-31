/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : message_bus.c
 * @Author       : Codex
 * @Date         : 2026-03-20 19:45:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 19:45:00
 * @Brief        : 轻量本地消息总线实现
 */

/*---------- includes ----------*/
#include "message_bus.h"
#include "options.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
struct msgbus_topic {
    const char *name;
    message_bus_t bus;
    struct list_head bus_node;
    struct list_head subscribers;
};

struct msgbus_topic_subscriber {
    msgbus_topic_t topic;
    msgbus_node_t owner;
    msgbus_topic_callback_t cb;
    void *user_data;
    struct list_head topic_node;
    struct list_head owner_node;
};

struct msgbus_service {
    const char *service;
    msgbus_node_t owner;
    msgbus_service_callback_t cb;
    void *user_data;
    struct list_head bus_node;
    struct list_head owner_node;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/* 统一的名称匹配入口，后续若引入静态字符串池可只改这一处。 */
static bool inline _msgbus_match_name(const char *lhs, const char *rhs);
static msgbus_topic_t _msgbus_find_topic(message_bus_t bus, const char *name);
static struct msgbus_topic_subscriber *_msgbus_find_topic_subscription(msgbus_node_t node, msgbus_topic_t topic);
static msgbus_service_t _msgbus_find_service(message_bus_t bus, const char *service);
static void _msgbus_clear_node_topics(msgbus_node_t node);
static void _msgbus_clear_node_services(msgbus_node_t node);
static void _msgbus_clear_bus_topics(message_bus_t bus);
/*---------- variable ----------*/
/*---------- function ----------*/
static bool inline _msgbus_match_name(const char *lhs, const char *rhs)
{
    if ((lhs == NULL) || (rhs == NULL)) {
        return false;
    }

    return (strcmp(lhs, rhs) == 0);
}

static msgbus_topic_t _msgbus_find_topic(message_bus_t bus, const char *name)
{
    msgbus_topic_t topic = NULL;

    if ((bus == NULL) || (name == NULL)) {
        return NULL;
    }

    list_for_each_entry(topic, struct msgbus_topic, &bus->topic_pool, bus_node)
    {
        if (_msgbus_match_name(topic->name, name)) {
            return topic;
        }
    }

    return NULL;
}

static struct msgbus_topic_subscriber *_msgbus_find_topic_subscription(msgbus_node_t node, msgbus_topic_t topic)
{
    struct msgbus_topic_subscriber *item = NULL;

    if ((node == NULL) || (topic == NULL)) {
        return NULL;
    }

    list_for_each_entry(item, struct msgbus_topic_subscriber, &node->topics, owner_node)
    {
        if (item->topic == topic) {
            return item;
        }
    }

    return NULL;
}

static msgbus_service_t _msgbus_find_service(message_bus_t bus, const char *service)
{
    msgbus_service_t item = NULL;

    if ((bus == NULL) || (service == NULL)) {
        return NULL;
    }

    list_for_each_entry(item, struct msgbus_service, &bus->service_pool, bus_node)
    {
        if (_msgbus_match_name(item->service, service)) {
            return item;
        }
    }

    return NULL;
}

static void _msgbus_clear_node_topics(msgbus_node_t node)
{
    struct msgbus_topic_subscriber *item = NULL;
    struct msgbus_topic_subscriber *next = NULL;

    /* 节点退出时，需要同时从节点私有链表和总线总表中摘除订阅项。 */
    list_for_each_entry_safe(item, next, struct msgbus_topic_subscriber, &node->topics, owner_node)
    {
        list_del(&item->owner_node);
        list_del(&item->topic_node);
        free(item);
    }
}

static void _msgbus_clear_node_services(msgbus_node_t node)
{
    msgbus_service_t item = NULL;
    msgbus_service_t next = NULL;

    /* 服务项同样挂在两条链表上，释放时必须成对摘除。 */
    list_for_each_entry_safe(item, next, struct msgbus_service, &node->services, owner_node)
    {
        list_del(&item->owner_node);
        list_del(&item->bus_node);
        free(item);
    }
}

static void _msgbus_clear_bus_topics(message_bus_t bus)
{
    msgbus_topic_t topic = NULL;
    msgbus_topic_t next = NULL;

    list_for_each_entry_safe(topic, next, struct msgbus_topic, &bus->topic_pool, bus_node)
    {
        list_del(&topic->bus_node);
        free(topic);
    }
}

void msgbus_init(message_bus_t bus, const char *name)
{
    assert(bus);
    assert(name);

    /* 总线对象由外部持有，这里只负责清零并初始化各类注册池。 */
    memset(bus, 0, sizeof(*bus));
    bus->name = name;
    INIT_LIST_HEAD(&bus->node_pool);
    INIT_LIST_HEAD(&bus->topic_pool);
    INIT_LIST_HEAD(&bus->service_pool);
}

void msgbus_deinit(message_bus_t bus)
{
    struct msgbus_node *node = NULL;
    struct msgbus_node *next = NULL;

    assert(bus);

    /* 先逐个反初始化节点，确保节点下挂的 topic/service 全部释放。 */
    list_for_each_entry_safe(node, next, struct msgbus_node, &bus->node_pool, node)
    {
        msgbus_node_deinit(node);
    }

    _msgbus_clear_bus_topics(bus);

    memset(bus, 0, sizeof(*bus));
    INIT_LIST_HEAD(&bus->node_pool);
    INIT_LIST_HEAD(&bus->topic_pool);
    INIT_LIST_HEAD(&bus->service_pool);
}

bool msgbus_node_init(msgbus_node_t node, message_bus_t bus, const char *name)
{
    if ((node == NULL) || (bus == NULL) || (name == NULL)) {
        return false;
    }
    /* 同一条总线上节点名必须唯一，避免后续按名查找出现歧义。 */
    if (msgbus_node_find(bus, name) != NULL) {
        return false;
    }

    memset(node, 0, sizeof(*node));
    node->name = name;
    node->bus = bus;
    INIT_LIST_HEAD(&node->topics);
    INIT_LIST_HEAD(&node->services);
    list_add_tail(&node->node, &bus->node_pool);

    return true;
}

void msgbus_node_deinit(msgbus_node_t node)
{
    if (node == NULL) {
        return;
    }

    /* 先清理节点拥有的订阅和服务，再把节点自身从总线中摘掉。 */
    _msgbus_clear_node_topics(node);
    _msgbus_clear_node_services(node);

    if ((node->node.next != NULL) && (node->node.prev != NULL)) {
        list_del(&node->node);
    }

    memset(node, 0, sizeof(*node));
}

msgbus_node_t msgbus_node_find(message_bus_t bus, const char *name)
{
    struct msgbus_node *node = NULL;

    if ((bus == NULL) || (name == NULL)) {
        return NULL;
    }

    list_for_each_entry(node, struct msgbus_node, &bus->node_pool, node)
    {
        if (_msgbus_match_name(node->name, name)) {
            return node;
        }
    }

    return NULL;
}

msgbus_topic_t msgbus_topic_find(message_bus_t bus, const char *name)
{
    return _msgbus_find_topic(bus, name);
}

bool msgbus_topic_register(msgbus_topic_t *topic, message_bus_t bus, const char *name)
{
    msgbus_topic_t item = NULL;

    if ((topic == NULL) || (bus == NULL) || (name == NULL)) {
        return false;
    }

    item = _msgbus_find_topic(bus, name);
    if (item != NULL) {
        *topic = item;
        return true;
    }

    item = malloc(sizeof(*item));
    if (item == NULL) {
        return false;
    }

    memset(item, 0, sizeof(*item));
    item->name = name;
    item->bus = bus;
    INIT_LIST_HEAD(&item->subscribers);
    list_add_tail(&item->bus_node, &bus->topic_pool);

    *topic = item;
    return true;
}

bool msgbus_topic_subscribe(msgbus_node_t node, msgbus_topic_t topic, msgbus_topic_callback_t cb, void *user_data)
{
    struct msgbus_topic_subscriber *item = NULL;

    if ((node == NULL) || (topic == NULL) || (node->bus == NULL) || (topic->bus != node->bus) || (cb == NULL)) {
        return false;
    }
    if (_msgbus_find_topic_subscription(node, topic) != NULL) {
        return false;
    }

    item = malloc(sizeof(*item));
    if (item == NULL) {
        return false;
    }

    memset(item, 0, sizeof(*item));
    item->topic = topic;
    item->owner = node;
    item->cb = cb;
    item->user_data = user_data;
    /* 订阅项同时挂到总线级索引和节点私有索引，方便两种方向遍历。 */
    list_add_tail(&item->topic_node, &topic->subscribers);
    list_add_tail(&item->owner_node, &node->topics);

    return true;
}

uint32_t msgbus_topic_publish(msgbus_topic_t topic, const void *msg, uint32_t size)
{
    struct msgbus_topic_subscriber *item = NULL;
    uint32_t count = 0U;

    if ((topic == NULL) || (msg == NULL) || (size == 0U)) {
        return 0U;
    }

    /* 发布时只按 topic 名广播，不建立发布者与订阅者之间的反向关系。 */
    list_for_each_entry(item, struct msgbus_topic_subscriber, &topic->subscribers, topic_node)
    {
        item->cb(item->owner, topic->name, msg, size, item->user_data);
        count++;
    }

    return count;
}

bool msgbus_service_advertise(
    msgbus_service_t *service, msgbus_node_t node, const char *name, msgbus_service_callback_t cb, void *user_data)
{
    msgbus_service_t item = NULL;

    if ((service == NULL) || (node == NULL) || (node->bus == NULL) || (name == NULL) || (cb == NULL)) {
        return false;
    }
    if (_msgbus_find_service(node->bus, name) != NULL) {
        return false;
    }

    item = malloc(sizeof(*item));
    if (item == NULL) {
        return false;
    }

    memset(item, 0, sizeof(*item));
    item->service = name;
    item->owner = node;
    item->cb = cb;
    item->user_data = user_data;
    /* 服务名在总线上全局唯一，调用方按名称直接路由到唯一处理者。 */
    list_add_tail(&item->bus_node, &node->bus->service_pool);
    list_add_tail(&item->owner_node, &node->services);

    *service = item;
    return true;
}

msgbus_service_t msgbus_service_find(message_bus_t bus, const char *name)
{
    return _msgbus_find_service(bus, name);
}

int32_t
msgbus_service_call(msgbus_service_t service, const void *req, uint32_t req_size, void *resp, uint32_t *resp_size)
{
    if ((service == NULL) || (req == NULL) || (req_size == 0U)) {
        return -MSGBUS_ERR_INVALID_ARGS;
    }
    if (service->cb == NULL) {
        return -MSGBUS_ERR_NO_HANDLER;
    }

    /* service 调用是点对点请求，不做缓存和排队，直接同步进入处理函数。 */
    return service->cb(service->owner, req, req_size, resp, resp_size, service->user_data);
}
/*---------- end of file ----------*/
