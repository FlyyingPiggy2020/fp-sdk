/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at_parser.c
 * @Author       : lxf
 * @Date         : 2024-11-07 09:43:47
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-11-08 14:20:00
 * @Brief        : at指令解析器
 */

/*---------- includes ----------*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "at_core.h"
#include "at_cmd_register.h"

#if defined(CONF_FPSDK_ENABLE_LOG)
#undef LOG_TAG
#define LOG_TAG "AT_PARSER"
#include "log_port.h"
#endif

#include "FreeRTOS.h"
#include "task.h"

/*---------- macro ----------*/
/*---------- type define ----------*/

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief 判断是否是at指令集"开头是AT+"，结尾是\\r\\n
 * @param {char} *recv_buf
 * @param {uint16_t} recv_len
 * @return {*}
 */
bool _is_at_cmd(uint8_t *rxbuf, uint16_t rxlen)
{
    char *AT = "AT+";
    char *LR = "\r\n";
    if (rxlen >= 5 && !memcmp(rxbuf, AT, strlen(AT)) && !memcmp(rxbuf + rxlen - strlen(LR), LR, strlen(LR))) {
        return true;
    }
    return false;
}

/**
 * @brief 判断at指令类型
 * @param {uint8_t} *recv_buf
 * @param {uint16_t} recv_len
 * @return {*}
 */
at_cmd_type_t identify_at_command_type(uint8_t *recv_buf, uint16_t recv_len)
{
    // 检查是否为测试命令 AT+<cmd>=?
    if (recv_buf[recv_len - 4] == '=' && recv_buf[recv_len - 3] == '?' && recv_buf[recv_len - 2] == '\r') {
        return ATCMD_TYPE_TEST;
    }

    // 检查是否为查询命令 AT+<cmd>?
    if (recv_buf[recv_len - 3] == '?' && recv_buf[recv_len - 2] == '\r') {
        return ATCMD_TYPE_READ;
    }

    // 检查是否为设置命令 AT+<cmd>=<param>
    if (memchr(recv_buf, '=', recv_len) != NULL) {
        return ATCMD_TYPE_SET;
    }
    // 默认返回执行命令类型
    return ATCMD_TYPE_EXECUTE;
}

/**
 * @brief at子命令解析
 * @param {uint8_t} *recv_buf
 * @param {uint16_t} recv_len
 * @param {at_cmd_set_register_t} *fn_ptr
 * @return {*}
 */
at_error_code at_cmd_executor(uint8_t *recv_buf, uint16_t recv_len, at_cmd_set_register_t *fn_ptr)
{
    at_cmd_type_t format = identify_at_command_type(recv_buf, recv_len);
    int argc = 0;
    void **argv = NULL;
    at_error_code ret = AT_SUB_OK;

    /* 对于设置命令 AT+<cmd>=<p1>,<p2>，查询argc和**argv */
    if (format == ATCMD_TYPE_SET) {
        /* 将\r\n替换为\0 */
        recv_buf[recv_len - 2] = '\0';
        recv_buf[recv_len - 1] = '\0';
        void *param = strtok((char *)recv_buf, "="); //按照"="分割字符串
        if (param != NULL) {
            param++;                   //跳过'='字符
            param = strtok(NULL, ","); //按照","分割字符串
            while (param != NULL) {
                argv = realloc(argv, sizeof(void *) * (argc + 1));
                if (argv == NULL) {
                    ret = AT_ERROR_MEMORY_ALLOCATION;
                    goto back;
                }
                argv[argc++] = param;
                param = strtok(NULL, ",");
            }
        }
    }

    for (uint16_t i = 0; i < fn_ptr->list_number; i++) {
        if (!memcmp(recv_buf, fn_ptr->sub_cmd_list[i].at_cmdName, strlen(fn_ptr->sub_cmd_list[i].at_cmdName))) {
            if (fn_ptr->sub_cmd_list[i].at_cmd_func != NULL) {
                ret = fn_ptr->sub_cmd_list[i].at_cmd_func(format, argc, argv);
                goto back;
            }
        }
    }
    ret = AT_SUB_UNSUPPORT_CMD;
back:
    if (argv != NULL) {
        free(argv); // 确保释放内存
    }
    return ret;
}
/**
 * @brief 从接收的数据中找到对应的at指令 这里的数据包头已经是AT+，而包尾是\r\n
 * @param {uint8_t} *recv_buf
 * @param {uint16_t} recv_len
 * @return {*}
 */
at_error_code at_cmd_parser(uint8_t *recv_buf, uint16_t recv_len)
{
    extern int _at_cmd_start;
    extern int _at_cmd_end;
    volatile at_cmd_set_register_t *fn_ptr;
    at_error_code ret = AT_SUB_OK;
    TickType_t start_time, end_time;
    start_time = xTaskGetTickCount();
    do {
        if (_is_at_cmd(recv_buf, recv_len) == false) {
            ret = AT_SUB_NO_AT;
            goto back;
        }

        for (fn_ptr = (void *)(&_at_cmd_start); fn_ptr < (void *)(&_at_cmd_end); fn_ptr++) {
            if (!memcmp(recv_buf, fn_ptr->cmd_name, strlen(fn_ptr->cmd_name))) {
                ret = at_cmd_executor(recv_buf, recv_len, fn_ptr);
            }
        }
    } while (0);
back:
    end_time = xTaskGetTickCount();
    char *error_s[] = { "ok", "no at", "unsupport", "error", "op error" };
    log_i("at_parser cost %d ms. ret:%s.", end_time - start_time, error_s[ret]);
    return ret;
}
/*---------- end of file ----------*/
