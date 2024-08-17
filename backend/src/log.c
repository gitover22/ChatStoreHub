#include "log.h"
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// 获取当前日期并格式化为字符串
void get_current_date(char *buffer, size_t buffer_size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, buffer_size, "%Y-%m-%d", t);
}

// 初始化日志文件，创建以日期和文件名命名的日志文件
int init_log(LogHandle *handle, const char *filename) {
    if (!filename || strlen(filename) == 0) {
        return -1; // 如果文件名为空或无效，返回错误
    }

    char date[20];
    get_current_date(date, sizeof(date));

    // 构造日志文件路径和名称
    snprintf(handle->filename, sizeof(handle->filename), "/home/huafeng/ChatStoreHub/backend/log/%s_%s_log.txt", date, filename);

    // 打开日志文件
    handle->file = fopen(handle->filename, "a");
    if (!handle->file) {
        return -1;
    }
    return 0;
}

// 写日志信息到日志文件
void write_log(LogHandle *handle, const char *format, ...) {
    if (!handle->file) {
        return;
    }

    va_list args;
    va_start(args, format);

    // 写入日志信息
    vfprintf(handle->file, format, args);
    fprintf(handle->file, "\n");
    fflush(handle->file);

    va_end(args);
}

// 关闭日志文件
void close_log(LogHandle *handle) {
    if (handle->file) {
        fclose(handle->file);
        handle->file = NULL;
    }
}
