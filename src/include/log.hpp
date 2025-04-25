#pragma once

// 定义日志文件
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>




// 1. 日志宏的定义

#define LDBG 0
#define LINF 1
#define LERR 2

#define LDFAULT LDBG

inline std::mutex global_log_lock;
// 定义日志字段
#define LOG(level, format, ...)                                                                 \
    {                                                                                           \
        std::unique_lock<std::mutex>                                                            \
            lock(global_log_lock);                                                              \
        if (level >= LDFAULT)                                                                   \
        {                                                                                       \
            time_t current_time;                                                                \
            time(&current_time);                                                                \
            struct tm *curtime = localtime(&current_time);                                      \
            char format_str[32] = {0};                                                          \
            strftime(format_str, 31, "%m-%d %T", curtime);                                      \
            printf("[%s][%s:%d]: " format "\n", format_str, __FILE__, __LINE__, ##__VA_ARGS__); \
        }                                                                                       \
    }

#define DLOG(format, ...) LOG(LDBG, format, ##__VA_ARGS__)
#define ILOG(format, ...) LOG(LINF, format, ##__VA_ARGS__)
#define ELOG(format, ...) LOG(LERR, format, ##__VA_ARGS__)
