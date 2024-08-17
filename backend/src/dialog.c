#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

#define PORT 10001

struct connection_info_struct {
    char *post_data;
    size_t post_data_len;
    struct MHD_PostProcessor *post_processor;
};

/**
 * @brief 发送HTTP响应
 * 
 * @param connection MHD连接对象，标识与客户端的连接。
 * @param message 响应消息体的内容，通常是HTML文本。
 * @param status_code 响应状态码，如200表示成功。
 * 
 * @return 返回MHD的处理结果，通常用于判断响应是否成功发送。
 */
int send_response(struct MHD_Connection *connection, const char *message, int status_code) {
    struct MHD_Response *response;
    int ret;

    // 创建HTTP响应对象
    response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain; charset=UTF-8");

    // 发送响应
    ret = MHD_queue_response(connection, status_code, response);

    // 销毁响应对象
    MHD_destroy_response(response);

    return ret;
}

/**
 * @brief 处理POST请求中的参数
 */
int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                 const char *filename, const char *content_type,
                 const char *transfer_encoding, const char *data, uint64_t off, size_t size) {
    struct connection_info_struct *con_info = coninfo_cls;

    if (strcmp(key, "question") == 0) {
        char *new_data = realloc(con_info->post_data, con_info->post_data_len + size + 1);
        if (new_data == NULL)
            return MHD_NO;

        con_info->post_data = new_data;
        memcpy(&(con_info->post_data[con_info->post_data_len]), data, size);
        con_info->post_data_len += size;
        con_info->post_data[con_info->post_data_len] = '\0';
    }

    return MHD_YES;
}

/**
 * @brief 请求处理函数，用于处理来自客户端的HTTP请求
 */
int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method, const char *version,
                    const char *upload_data, size_t *upload_data_size, void **con_cls) {
    if (NULL == *con_cls) {
        struct connection_info_struct *con_info = malloc(sizeof(struct connection_info_struct));
        if (NULL == con_info)
            return MHD_NO;

        con_info->post_data = NULL;
        con_info->post_data_len = 0;
        con_info->post_processor = MHD_create_post_processor(connection, 1024, iterate_post, (void *)con_info);

        *con_cls = (void *)con_info;

        return MHD_YES;
    }

    if (strcmp(method, "POST") == 0) {
        struct connection_info_struct *con_info = *con_cls;

        if (*upload_data_size != 0) {
            MHD_post_process(con_info->post_processor, upload_data, *upload_data_size);
            *upload_data_size = 0;

            return MHD_YES;
        } else {
            // 打印用户输入到终端
            printf("Received question: %s\n", con_info->post_data);

            // 生成一个简单的响应
            const char *response_msg = "This is a response from the server.";
            printf("Sending response: %s\n", response_msg);

            return send_response(connection, response_msg, MHD_HTTP_OK);
        }
    }

    return send_response(connection, "Bad Request", MHD_HTTP_BAD_REQUEST);
}

/**
 * @brief 请求完成后调用的清理函数
 */
void request_completed(void *cls, struct MHD_Connection *connection, void **con_cls,
                       enum MHD_RequestTerminationCode toe) {
    struct connection_info_struct *con_info = *con_cls;

    if (con_info != NULL) {
        if (con_info->post_processor != NULL)
            MHD_destroy_post_processor(con_info->post_processor);

        if (con_info->post_data != NULL)
            free(con_info->post_data);

        free(con_info);
    }

    *con_cls = NULL;
}

int main(int argc, char **argv) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);

    if (daemon == NULL)
        return 1;

    printf("Server is running on port %d\n", PORT);
    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
