#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// 日志句柄
typedef struct {
    FILE *file;
    char filename[256];
} LogHandle;

// 日志模块初始化，传入源文件名用于生成日志文件名
int init_log(LogHandle *handle, const char *filename);

// 写日志接口
void write_log(LogHandle *handle, const char *format, ...);

// 关闭日志文件
void close_log(LogHandle *handle);

#endif // LOG_H
