/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at_core.h
 * @Author       : lxf
 * @Date         : 2024-11-06 16:55:23
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-11-08 14:19:17
 * @Brief        :
 */

#pragma once
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
/*---------- macro ----------*/

#define at_min(x, y) ((x) < (y) ? (x) : (y))
#define at_max(x, y) ((x) > (y) ? (x) : (y))
/*---------- type define ----------*/

/**
 * @brief at_status
 *  some custom function interacting with AT
 *
 */
typedef enum {
    AT_STATUS_NORMAL = 0x0, /*!< Normal mode.Now mcu can send AT command */
    AT_STATUS_TRANSMIT,     /*!< Transparent Transmition mode */
} at_status_type;

/**
 * @brief subcategory number
 *
 */
typedef enum {
    AT_SUB_OK = 0x00,                  /*!< OK */
    AT_SUB_NO_AT = 0x01,               /*!< Starting "AT" not found (or at, At or aT entered) or terminator character not found ("\r\n" expected)*/
    AT_SUB_UNSUPPORT_CMD = 0x02,       /*!< the command is not supported */
    AT_SUB_COMMON_ERROR = 0x03,        /*!< reserved */
    AT_SUB_CMD_OP_ERROR = 0x04,        /*!< the command operation type is error */
    AT_ERROR_MEMORY_ALLOCATION = 0x05, /*!< memory allocation failed */
    AT_SUB_PARA_INVALID = 0x06,        /*!< the parameter is invalid */

    // AT_SUB_NO_TERMINATOR = 0x02,        /*!< terminator character not found ("\r\n" expected) */
    // AT_SUB_NO_AT = 0x03,                /*!< Starting "AT" not found (or at, At or aT entered) */
    // AT_SUB_PARA_LENGTH_MISMATCH = 0x04, /*!< parameter length mismatch */
    // AT_SUB_PARA_TYPE_MISMATCH = 0x05,   /*!< parameter type mismatch */
    // AT_SUB_PARA_NUM_MISMATCH = 0x06,    /*!< parameter number mismatch */
    // AT_SUB_PARA_PARSE_FAIL = 0x08,      /*!< parse parameter fail */
    // AT_SUB_UNSUPPORT_CMD = 0x09,        /*!< the command is not supported */
    // AT_SUB_CMD_EXEC_FAIL = 0x0A,        /*!< the command execution failed */
    // AT_SUB_CMD_PROCESSING = 0x0B,       /*!< processing of previous command is in progress */
} at_error_code;

/**
 * @brief the result of AT parse
 *
 */
typedef enum {
    AT_PARA_PARSE_RESULT_FAIL = -1, /*!< parse fail,Maybe the type of parameter is mismatched,or out of range */
    AT_PARA_PARSE_RESULT_OK = 0,    /*!< Successful */
    AT_PARA_PARSE_RESULT_OMITTED,   /*!< the parameter is OMITTED. */
} esp_at_para_parse_result_type;

/**
 * @brief the result code of AT command processing
 *
 */
typedef enum {
    AT_RESULT_CODE_OK = 0x00,                  /*!< "OK" */
    AT_RESULT_CODE_ERROR = 0x01,               /*!< "ERROR" */
    AT_RESULT_CODE_FAIL = 0x02,                /*!< "ERROR" */
    AT_RESULT_CODE_SEND_OK = 0x03,             /*!< "SEND OK" */
    AT_RESULT_CODE_SEND_FAIL = 0x04,           /*!< "SEND FAIL" */
    AT_RESULT_CODE_IGNORE = 0x05,              /*!< response nothing, just change internal status */
    AT_RESULT_CODE_PROCESS_DONE = 0x06,        /*!< response nothing, just change internal status */
    AT_RESULT_CODE_OK_AND_INPUT_PROMPT = 0x07, // "OK" and ">"
    AT_RESULT_CODE_MAX
} at_result_code_string_index;

typedef enum {
    ATCMD_TYPE_TEST = 0, // 测试命令 AT+<cmd>=?
    ATCMD_TYPE_READ,     // 查询命令 AT+<cmd>?
    ATCMD_TYPE_SET,      // 设置命令 AT+<cmd>=<p1>...
    ATCMD_TYPE_EXECUTE   // 执行命令 AT+<cmd>
} at_cmd_type_t;

/**
 * @brief at_cmd_struct
 *  used for define at command
 *
 */
typedef struct {
    char *at_cmdName; /*!< at command name */
    at_error_code (*at_cmd_func)(at_cmd_type_t type, int argc, void *argv[]);
} at_cmd_struct;
/*---------- variable prototype ----------*/

bool at_custom_cmd_array_regist(const at_cmd_struct *custom_at_cmd_array, uint32_t cmd_num);
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
