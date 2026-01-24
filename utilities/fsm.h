/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : fsm.h
 * @Author       : lxf
 * @Date         : 2026-01-06 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-06 10:00:00
 * @Brief        : 通用型有限状态机框架
 * @features     :
 *               - 状态支持进入/退出/轮询回调
 *               - 最小停留时间防止状态快速切换
 *               - 条件驱动的状态转移
 *               - 时间片驱动,支持自定义时基
 * @usage        :
 *               @code
 *               // 创建并配置状态机
 *               struct fsm *fsm = fsm_create(&user_context);
 *               struct fsm_state *s1 = fsm_add_state(fsm, "State1", 0, enter_cb, exit_cb, poll_cb);
 *               struct fsm_state *s2 = fsm_add_state(fsm, "State2", 0, NULL, NULL, NULL);
 *               fsm_add_transition(fsm, s1, s2, "Event", criteria_func);
 *               fsm_start(fsm, s1);
 *
 *               // 定时器中断 (1ms~10ms) - 时间步进
 *               void SysTick_Handler(void) {
 *                   fsm_tick(fsm, 1);  // 累加停留时间
 *               }
 *
 *               // 主循环 - 逻辑执行 (全速调用)
 *               while (1) {
 *                   fsm_execute(fsm);  // 执行poll并检查转移
 *               }
 *
 *               fsm_destroy(fsm);
 *               @endcode
 * @note         :
 *               - 状态名称使用字符串常量,框架不管理名称内存
 *               - fsm_tick处理时间,通常在定时器中调用(低频)
 *               - fsm_execute处理逻辑,可在主循环中全速调用(高频)
 *               - 两者解耦实现最佳响应性能
 */

#ifndef __FSM_H__
#define __FSM_H__
#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include "options.h"

/*---------- macro ----------*/
/**
 * @brief  FSM日志输出宏
 * @note   可通过重新定义此宏来改变日志输出方式
 */
#ifndef fsm_log
#define fsm_log(...) xlog_error(__VA_ARGS__)
#endif

/*---------- type define ----------*/
/* 前置声明 */
struct fsm;
struct fsm_state;
struct fsm_transition;

/**
 * @brief  状态机结构体
 */
struct fsm {
    struct list_head states;      /* 所有状态的链表 */
    struct fsm_state *current;    /* 当前状态 */
    struct fsm_state *initial;    /* 初始状态 */
    bool is_running;              /* 运行状态标志 */
    void *user_context;           /* 用户上下文，传递给回调函数 */
};

/**
 * @brief  状态结构体
 */
struct fsm_state {
    struct list_head list;           /* 链表节点，挂载到fsm->states */
    struct list_head transitions;    /* 从此状态出发的转移链表 */
    const char *name;                /* 状态名称(用于日志) */
    uint32_t min_time_ms;            /* 最小停留时间(毫秒) */
    uint32_t accumulated_ms;         /* 已停留累计时间 */
    void (*on_enter)(void *context); /* 进入回调 */
    void (*on_exit)(void *context);  /* 退出回调 */
    void (*on_poll)(void *context);  /* 轮询回调，每次update时调用 */
};

/**
 * @brief  转移结构体
 */
struct fsm_transition {
    struct list_head list;           /* 链表节点 */
    struct fsm_state *source;        /* 源状态 */
    struct fsm_state *dest;          /* 目标状态 */
    const char *event_name;          /* 转移事件名称(用于日志) */
    bool (*criteria)(void *context); /* 转移条件函数 */
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  创建状态机实例
 * @param  user_context: 用户上下文指针，传递给回调函数
 * @return 状态机实例指针，失败返回NULL
 */
struct fsm *fsm_create(void *user_context);

/**
 * @brief  销毁状态机实例
 * @param  fsm: 状态机实例指针
 * @return 无
 */
void fsm_destroy(struct fsm *fsm);

/**
 * @brief  添加状态
 * @param  fsm: 状态机实例指针
 * @param  name: 状态名称
 * @param  min_time_ms: 最小停留时间(毫秒)
 * @param  on_enter: 进入状态时的回调函数
 * @param  on_exit: 退出状态时的回调函数
 * @param  on_poll: 轮询回调函数，每次update时调用
 * @return 状态指针，失败返回NULL
 */
struct fsm_state *fsm_add_state(struct fsm *fsm,
                                const char *name,
                                uint32_t min_time_ms,
                                void (*on_enter)(void *),
                                void (*on_exit)(void *),
                                void (*on_poll)(void *));

/**
 * @brief  添加转移条件
 * @param  fsm: 状态机实例指针
 * @param  source: 源状态
 * @param  dest: 目标状态
 * @param  event_name: 转移事件名称(用于日志)
 * @param  criteria: 转移条件判断函数
 * @return 0=成功, <0=失败
 * @note   转移条件按添加顺序判断(FIFO)，先添加的优先判断。
 *         若多个条件同时满足，将触发第一个添加的转移。
 */
int fsm_add_transition(struct fsm *fsm,
                       struct fsm_state *source,
                       struct fsm_state *dest,
                       const char *event_name,
                       bool (*criteria)(void *));

/**
 * @brief  启动状态机
 * @param  fsm: 状态机实例指针
 * @param  initial: 初始状态
 * @return 0=成功, <0=失败
 */
int fsm_start(struct fsm *fsm, struct fsm_state *initial);

/**
 * @brief  时间步进（低频调用，如定时器中断中）
 * @param  fsm: 状态机实例指针
 * @param  ms: 时间片(毫秒)，累加到当前状态的停留时间
 * @return 无
 * @note   用于处理最小停留时间计时，通常在SysTick(1ms~10ms)中调用
 */
void fsm_tick(struct fsm *fsm, uint32_t ms);

/**
 * @brief  逻辑执行（高频调用，如主循环中）
 * @param  fsm: 状态机实例指针
 * @return 0=无转移, 1=发生转移, <0=错误
 * @note   执行on_poll回调并检查转移条件，可在主循环中全速调用
 */
int fsm_execute(struct fsm *fsm);

/**
 * @brief  根据名称查找状态
 * @param  fsm: 状态机实例指针
 * @param  name: 状态名称
 * @return 状态指针，未找到返回NULL
 */
struct fsm_state *fsm_get_state_by_name(struct fsm *fsm, const char *name);

/**
 * @brief  获取当前状态名称
 * @param  fsm: 状态机实例指针
 * @return 当前状态名称，未启动返回NULL
 */
const char *fsm_get_current_state_name(struct fsm *fsm);

/**
 * @brief  检查状态机是否处于指定状态
 * @param  fsm: 状态机实例指针
 * @param  state: 要检查的状态
 * @return true=处于该状态, false=不处于该状态
 */
bool fsm_is_in_state(struct fsm *fsm, struct fsm_state *state);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif