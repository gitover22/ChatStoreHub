#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <mysql/mysql.h>

#define PORT 10000
#define DB_HOST "localhost"
#define DB_USER "huafeng"
#define DB_PASS "huafeng"
#define DB_NAME "CSH"

struct connection_info_struct {
    char *post_data;
    size_t post_data_len;
    struct MHD_PostProcessor *post_processor;
};

int check_user(const char *username, const char *password) {
    printf("Checking user: %s, password: %s\n", username, password);
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return 0;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        return 0;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username='%s' AND password='%s'", username, password);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed\n");
        mysql_close(conn);
        return 0;
    }

    int num_rows = mysql_num_rows(res);
    mysql_free_result(res);
    mysql_close(conn);

    return num_rows > 0;
}

int send_response(struct MHD_Connection *connection, const char *message, int status_code) {
    struct MHD_Response *response;
    int ret;

    // 添加Content-Type头信息，指定UTF-8编码
    response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                 const char *filename, const char *content_type,
                 const char *transfer_encoding, const char *data, uint64_t off, size_t size) {
    struct connection_info_struct *con_info = coninfo_cls;

    if (strcmp(key, "username") == 0 || strcmp(key, "password") == 0) {
        char *new_data = realloc(con_info->post_data, con_info->post_data_len + size + 1);
        if (new_data == NULL) {
            return MHD_NO;
        }

        con_info->post_data = new_data;
        memcpy(&(con_info->post_data[con_info->post_data_len]), data, size);
        con_info->post_data_len += size;
        con_info->post_data[con_info->post_data_len] = '\0';
    }

    return MHD_YES;
}

int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method, const char *version,
                    const char *upload_data, size_t *upload_data_size, void **con_cls) {
    if (NULL == *con_cls) {
        struct connection_info_struct *con_info = malloc(sizeof(struct connection_info_struct));
        if (NULL == con_info) {
            return MHD_NO;
        }
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
            char username[50];
            char password[50];

            sscanf(con_info->post_data, "username=%49[^&]&password=%49s", username, password);

            if (check_user(username, password)) {
                const char *success_page = "<html><body><h1>登录成功</h1><p>欢迎回来，用户！</p></body></html>";
                return send_response(connection, success_page, MHD_HTTP_OK);
            } else {
                const char *fail_page = "<html><body><h1>登录失败</h1><p>用户名或密码错误，请重试。</p></body></html>";
                return send_response(connection, fail_page, MHD_HTTP_UNAUTHORIZED);
            }
        }
    }

    const char *bad_request_page = "<html><body><h1>错误请求</h1><p>请求无效，请使用POST方法提交数据。</p></body></html>";
    return send_response(connection, bad_request_page, MHD_HTTP_BAD_REQUEST);
}

void request_completed(void *cls, struct MHD_Connection *connection, void **con_cls,
                       enum MHD_RequestTerminationCode toe) {
    struct connection_info_struct *con_info = *con_cls;
    if (con_info != NULL) {
        if (con_info->post_processor != NULL) {
            MHD_destroy_post_processor(con_info->post_processor);
        }
        if (con_info->post_data != NULL) {
            free(con_info->post_data);
        }
        free(con_info);
    }
    *con_cls = NULL;
}

int main(int argc ,char **argv) {
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
