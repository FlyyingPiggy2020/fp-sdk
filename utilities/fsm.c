/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : fsm.c
 * @Author       : lxf
 * @Date         : 2026-01-06 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-06 10:00:00
 * @Brief        : 通用型有限状态机框架实现
 * @features     :
 *               - 基于双向链表的状态和转移管理
 *               - 动态内存分配,支持运行时添加状态
 *               - 完整的日志记录,便于调试
 *               - 支持NULL回调,灵活配置
 * @note         :
 *               - 使用fp-sdk的clists实现链表管理
 *               - 内存分配使用fp-sdk的malloc/free
 *
 * @usage        :
 *               @code
 *               // ========== 1. 定义状态回调函数 ==========
 *               static void state_idle_enter(void *ctx) {
 *                   // 进入空闲状态时的处理
 *               }
 *               static void state_idle_poll(void *ctx) {
 *                   // 空闲状态下的轮询处理
 *               }
 *
 *               // ========== 2. 定义转移条件函数 ==========
 *               static bool check_start_button(void *ctx) {
 *                   // 返回true表示满足转移条件
 *                   return button_is_pressed();
 *               }
 *
 *               // ========== 3. 创建状态机 ==========
 *               struct fsm *fsm = fsm_create(NULL);
 *               if (!fsm) {
 *                   return -1;
 *               }
 *
 *               // ========== 4. 添加状态 ==========
 *               struct fsm_state *idle = fsm_add_state(fsm, "idle", 0,
 *                       state_idle_enter, NULL, state_idle_poll);
 *               struct fsm_state *running = fsm_add_state(fsm, "running", 1000,
 *                       NULL, NULL, NULL);
 *               struct fsm_state *stopped = fsm_add_state(fsm, "stopped", 0,
 *                       NULL, NULL, NULL);
 *
 *               // ========== 5. 添加转移条件 ==========
 *               fsm_add_transition(fsm, idle, running, "start", check_start_button);
 *               fsm_add_transition(fsm, running, stopped, "stop", check_stop_condition);
 *               fsm_add_transition(fsm, stopped, idle, "reset", check_reset_condition);
 *
 *               // ========== 6. 启动状态机 ==========
 *               fsm_start(fsm, idle);
 *
 *               // ========== 7. 在主循环中定期更新 ==========
 *               while (1) {
 *                   fsm_update(fsm, 10);  // 传入时间片(毫秒)
 *                   delay_ms(10);
 *               }
 *
 *               // ========== 8. 销毁状态机 ==========
 *               fsm_destroy(fsm);
 *               @endcode
 */

/*---------- includes ----------*/
#include "fsm.h"
#include <string.h>
#include <stddef.h>

/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief  创建状态机实例
 * @param  user_context: 用户上下文指针，传递给回调函数
 * @return 状态机实例指针，失败返回NULL
 */
struct fsm *fsm_create(void *user_context)
{
    struct fsm *fsm = NULL;

    fsm = (struct fsm *)malloc(sizeof(struct fsm));
    if (fsm == NULL) {
        fsm_log("FSM: malloc failed for fsm struct\r\n");
        return NULL;
    }

    /* 初始化链表 */
    INIT_LIST_HEAD(&fsm->states);

    /* 初始化成员变量 */
    fsm->current = NULL;
    fsm->initial = NULL;
    fsm->is_running = false;
    fsm->user_context = user_context;

    return fsm;
}

/**
 * @brief  销毁状态机实例
 * @param  fsm: 状态机实例指针
 * @return 无
 */
void fsm_destroy(struct fsm *fsm)
{
    struct fsm_state *state = NULL;
    struct fsm_state *state_tmp = NULL;
    struct fsm_transition *trans = NULL;
    struct fsm_transition *trans_tmp = NULL;

    if (fsm == NULL) {
        return;
    }

    /* 释放所有状态及其转移 */
    list_for_each_entry_safe(state, state_tmp, struct fsm_state, &fsm->states, list)
    {
        /* 释放该状态的所有转移 */
        list_for_each_entry_safe(trans, trans_tmp, struct fsm_transition, &state->transitions, list)
        {
            list_del(&trans->list);
            free(trans);
        }
        /* 释放状态 */
        list_del(&state->list);
        free(state);
    }

    /* 释放状态机本身 */
    free(fsm);
}

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
                                void (*on_poll)(void *))
{
    struct fsm_state *state = NULL;

    if (fsm == NULL || name == NULL) {
        fsm_log("FSM: invalid parameters for add_state\r\n");
        return NULL;
    }

    /* 检查状态名是否已存在 */
    if (fsm_get_state_by_name(fsm, name) != NULL) {
        fsm_log("FSM: state '%s' already exists\r\n", name);
        return NULL;
    }

    /* 分配状态结构体内存 */
    state = (struct fsm_state *)malloc(sizeof(struct fsm_state));
    if (state == NULL) {
        fsm_log("FSM: malloc failed for state '%s'\r\n", name);
        return NULL;
    }

    /* 初始化成员 */
    INIT_LIST_HEAD(&state->list);
    INIT_LIST_HEAD(&state->transitions);
    state->name = name;
    state->min_time_ms = min_time_ms;
    state->accumulated_ms = 0;
    state->on_enter = on_enter;
    state->on_exit = on_exit;
    state->on_poll = on_poll;

    /* 添加到状态链表 */
    list_add_tail(&state->list, &fsm->states);

    return state;
}

/**
 * @brief  添加转移条件
 * @param  fsm: 状态机实例指针
 * @param  source: 源状态
 * @param  dest: 目标状态
 * @param  event_name: 转移事件名称(用于日志)
 * @param  criteria: 转移条件判断函数
 * @return 0=成功, <0=失败
 */
int fsm_add_transition(
    struct fsm *fsm, struct fsm_state *source, struct fsm_state *dest, const char *event_name, bool (*criteria)(void *))
{
    struct fsm_transition *trans = NULL;

    if (fsm == NULL || source == NULL || dest == NULL || event_name == NULL) {
        fsm_log("FSM: invalid parameters for add_transition\r\n");
        return -1;
    }

    /* 分配转移结构体内存 */
    trans = (struct fsm_transition *)malloc(sizeof(struct fsm_transition));
    if (trans == NULL) {
        fsm_log("FSM: malloc failed for transition '%s'\r\n", event_name);
        return -2;
    }

    /* 初始化成员 */
    INIT_LIST_HEAD(&trans->list);
    trans->source = source;
    trans->dest = dest;
    trans->event_name = event_name;
    trans->criteria = criteria;

    /* 添加到源状态的转移链表 */
    list_add_tail(&trans->list, &source->transitions);

    return 0;
}

/**
 * @brief  启动状态机
 * @param  fsm: 状态机实例指针
 * @param  initial: 初始状态
 * @return 0=成功, <0=失败
 */
int fsm_start(struct fsm *fsm, struct fsm_state *initial)
{
    if (fsm == NULL || initial == NULL) {
        fsm_log("FSM: invalid parameters for start\r\n");
        return -1;
    }

    /* 检查初始状态是否属于此状态机 */
    if (fsm_get_state_by_name(fsm, initial->name) == NULL) {
        fsm_log("FSM: initial state '%s' not found in this fsm\r\n", initial->name);
        return -2;
    }

    fsm->initial = initial;
    fsm->current = initial;
    fsm->current->accumulated_ms = 0;
    fsm->is_running = true;

    /* 调用进入回调 */
    if (fsm->current->on_enter != NULL) {
        fsm->current->on_enter(fsm->user_context);
    }

    fsm_log("FSM: Enter state '%s'\r\n", fsm->current->name);

    return 0;
}

/**
 * @brief  时间步进（低频调用，如定时器中断中）
 * @param  fsm: 状态机实例指针
 * @param  ms: 时间片(毫秒)，累加到当前状态的停留时间
 * @return 无
 * @note   用于处理最小停留时间计时，通常在SysTick(1ms~10ms)中调用
 */
void fsm_tick(struct fsm *fsm, uint32_t ms)
{
    if (fsm == NULL || fsm->current == NULL) {
        return;
    }

    fsm->current->accumulated_ms += ms;
}

/**
 * @brief  逻辑执行（高频调用，如主循环中）
 * @param  fsm: 状态机实例指针
 * @return 0=无转移, 1=发生转移, <0=错误
 * @note   执行on_poll回调并检查转移条件，可在主循环中全速调用
 */
int fsm_execute(struct fsm *fsm)
{
    struct fsm_transition *trans = NULL;
    struct fsm_state *next_state = NULL;
    const char *event_name = NULL;

    if (fsm == NULL) {
        return -1;
    }

    /* 检查状态机是否已启动 */
    if (!fsm->is_running || fsm->current == NULL) {
        return -2;
    }

    /* 调用当前状态的轮询回调 */
    if (fsm->current->on_poll != NULL) {
        fsm->current->on_poll(fsm->user_context);
    }

    /* 检查是否满足最小停留时间 */
    if (fsm->current->accumulated_ms < fsm->current->min_time_ms) {
        return 0;
    }

    /* 遍历当前状态的转移，查找满足条件的转移 */
    list_for_each_entry(trans, struct fsm_transition, &fsm->current->transitions, list)
    {
        /* 评估转移条件 */
        if (trans->criteria != NULL && trans->criteria(fsm->user_context)) {
            next_state = trans->dest;
            event_name = trans->event_name;
            break;
        }
    }

    /* 如果没有找到满足条件的转移 */
    if (next_state == NULL) {
        return 0;
    }

    /* 执行状态转移 */

    /* 调用当前状态的退出回调 */
    if (fsm->current->on_exit != NULL) {
        fsm->current->on_exit(fsm->user_context);
    }

    fsm_log("FSM: Exit state '%s'\r\n", fsm->current->name);
    fsm_log("FSM: Transition '%s' -> '%s' via '%s'\r\n",
            fsm->current->name,
            next_state->name,
            event_name ? event_name : "unknown");

    /* 切换到新状态 */
    fsm->current = next_state;
    fsm->current->accumulated_ms = 0;

    /* 调用新状态的进入回调 */
    if (fsm->current->on_enter != NULL) {
        fsm->current->on_enter(fsm->user_context);
    }

    fsm_log("FSM: Enter state '%s'\r\n", fsm->current->name);

    return 1;
}

/**
 * @brief  根据名称查找状态
 * @param  fsm: 状态机实例指针
 * @param  name: 状态名称
 * @return 状态指针，未找到返回NULL
 */
struct fsm_state *fsm_get_state_by_name(struct fsm *fsm, const char *name)
{
    struct fsm_state *state = NULL;

    if (fsm == NULL || name == NULL) {
        return NULL;
    }

    list_for_each_entry(state, struct fsm_state, &fsm->states, list)
    {
        if (strcmp(state->name, name) == 0) {
            return state;
        }
    }

    return NULL;
}

/**
 * @brief  获取当前状态名称
 * @param  fsm: 状态机实例指针
 * @return 当前状态名称，未启动返回NULL
 */
const char *fsm_get_current_state_name(struct fsm *fsm)
{
    if (fsm == NULL || fsm->current == NULL) {
        return NULL;
    }

    return fsm->current->name;
}

/**
 * @brief  检查状态机是否处于指定状态
 * @param  fsm: 状态机实例指针
 * @param  state: 要检查的状态
 * @return true=处于该状态, false=不处于该状态
 */
bool fsm_is_in_state(struct fsm *fsm, struct fsm_state *state)
{
    if (fsm == NULL || state == NULL) {
        return false;
    }

    return (fsm->current == state);
}

/*---------- end of file ----------*/
