/*
 * Logging.h
 *
 *  Created on: 28 Jul 2017
 *      Author: pi
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <stdio.h>
#include <string.h>
#include <list>
#include "Logger.h"

extern Logger logger;
extern std::list<std::string> error_message_list;

#define MAX_ERR_MSG_LEN 4096
#define MAX_LOG_PREFIX_LEN 32

#define SOURCECODE_FILE_SHORT_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define log_internal(add_to_err_message_list, prefix, level, printf_format, ...) \
    do { \
        char __lerrmsg[MAX_ERR_MSG_LEN+1]; \
        snprintf(__lerrmsg, MAX_ERR_MSG_LEN, "%s", prefix); \
        size_t prefix_len = strlen(__lerrmsg); \
        snprintf(__lerrmsg+prefix_len, MAX_ERR_MSG_LEN-prefix_len, printf_format, __VA_ARGS__); \
        logger.output_line(level, SOURCECODE_FILE_SHORT_NAME, __LINE__, __lerrmsg); \
        if (add_to_err_message_list) { error_message_list.push_back(__lerrmsg); } \
        } while (0)

#define log_error(code, printf_format, ...) \
    do { \
        char __lprefix[MAX_LOG_PREFIX_LEN+1]; \
        sprintf_s(__lprefix, MAX_LOG_PREFIX_LEN, "(%d) ", code); \
        log_internal(true, __lprefix, LogLevel::ERROR_LEVEL, printf_format, __VA_ARGS__); \
        } while (0)

#define log_warning(printf_format, ...) log_internal(true, "", LogLevel::WARN_LEVEL,  printf_format, __VA_ARGS__)
#define log_info(printf_format, ...)    log_internal(false, "", LogLevel::INFO_LEVEL,  printf_format, __VA_ARGS__)
#define log_trace(printf_format, ...)   log_internal(false, "", LogLevel::TRACE_LEVEL, printf_format, __VA_ARGS__)
#define log_debug(printf_format, ...)   log_internal(false, "", LogLevel::DEBUG_LEVEL, printf_format, __VA_ARGS__)

#endif /* LOGGING_H_ */
