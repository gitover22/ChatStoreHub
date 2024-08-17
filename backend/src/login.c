#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <mysql/mysql.h>

#define PORT 10000
#define DB_HOST "127.0.0.1"
#define DB_USER "huafeng"
#define DB_PASS "huafeng"
#define DB_NAME "CSH"

struct connection_info_struct
{
    char *post_data;
    size_t post_data_len;
    struct MHD_PostProcessor *post_processor;
};

/**
 * @brief 检查用户是否存在于数据库中
 */
/**
 * 检查用户是否存在于数据库中
 *
 * @param username 用户名
 * @param password 密码
 * @return 如果用户存在返回1，否则返回0
 */
int check_user(const char *username, const char *password)
{
    // 打印正在检查的用户名和密码
    printf("Checking user: %s, password: %s\n", username, password);

    // 初始化MySQL连接对象
    MYSQL *conn;
    // 初始化结果集对象
    MYSQL_RES *res;
    // 初始化用于存储结果集行数据的变量
    MYSQL_ROW row;

    // 初始化MySQL连接，为NULL表示新建一个mysql对象
    conn = mysql_init(NULL);
    // 检查mysql初始化是否成功
    if (conn == NULL)
    {
        // 初始化失败，打印错误信息
        fprintf(stderr, "mysql_init() failed\n");
        // 返回错误
        return 0;
    }

    // 尝试连接到数据库
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL)
    {
        // 连接失败，打印错误信息
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        // 关闭数据库连接
        mysql_close(conn);
        // 返回错误
        return 0;
    }

    // 构造查询语句
    char query[256];
    // 安全地构造查询语句，避免SQL注入
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username='%s' AND password='%s'", username, password);

    // 执行查询，检查是否有错误
    if (mysql_query(conn, query))
    {
        // 查询错误，打印错误信息
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        // 关闭数据库连接
        mysql_close(conn);
        // 返回错误
        return 0;
    }

    // 获取查询结果集
    res = mysql_store_result(conn);
    // 检查结果集是否成功获取
    if (res == NULL)
    {
        // 获取失败，打印错误信息
        fprintf(stderr, "mysql_store_result() failed\n");
        // 关闭数据库连接
        mysql_close(conn);
        // 返回错误
        return 0;
    }

    // 获取结果集的行数
    int num_rows = mysql_num_rows(res);
    // 释放结果集内存
    mysql_free_result(res);
    // 关闭数据库连接
    mysql_close(conn);

    // 如果有匹配的行，则返回1，表示用户存在；否则返回0，表示用户不存在
    return num_rows > 0;
}

/**
 * 构建并发送HTTP响应。
 *
 * @param connection MHD连接对象，标识与客户端的连接。
 * @param message 响应消息体的内容，通常是HTML文本。
 * @param status_code 响应状态码，如200表示成功。
 *
 * @return 返回MHD的处理结果，通常用于判断响应是否成功发送。
 */
int send_response(struct MHD_Connection *connection, const char *message, int status_code)
{
    // 创建并初始化HTTP响应对象
    struct MHD_Response *response;
    int ret;

    // 添加Content-Type头信息，指定UTF-8编码
    response = MHD_create_response_from_buffer(strlen(message), (void *)message, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");

    // 将响应加入队列，等待发送
    ret = MHD_queue_response(connection, status_code, response);

    // 释放响应对象资源
    MHD_destroy_response(response);

    // 返回发送结果
    return ret;
}

/*
 * 处理POST请求中的参数。
 *
 * 该函数被调用以处理HTTP POST请求中的每个参数。它根据参数的类型和名称，
 * 将数据追加到连接信息结构体的post_data中。
 *
 * 参数：
 * - coninfo_cls: 指向连接信息的不透明指针。
 * - kind: 参数的类型，这里未使用。
 * - key: 参数的名称。
 * - filename: 如果参数是文件，这是文件名。此处未使用。
 * - content_type: 参数的内容类型。此处未使用。
 * - transfer_encoding: 参数的传输编码。此处未使用。
 * - data: 参数的值。
 * - off: 数据的偏移量。此处未使用。
 * - size: 要处理的数据的大小。
 *
 * 返回值：
 * - 如果成功处理数据，返回MHD_YES。
 * - 如果内存重新分配失败，返回MHD_NO。
 */
int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                 const char *filename, const char *content_type,
                 const char *transfer_encoding, const char *data, uint64_t off, size_t size)
{
    // 将传入的不透明指针转换为正确的类型
    struct connection_info_struct *con_info = coninfo_cls;

    // 检查参数名称是否为“username”或“password”
    if (strcmp(key, "username") == 0 || strcmp(key, "password") == 0)
    {
        // 为接收更多数据而扩展post_data的大小
        char *new_data = realloc(con_info->post_data, con_info->post_data_len + size + 1);
        if (new_data == NULL)
        {
            // 内存重新分配失败
            return MHD_NO;
        }

        // 更新post_data指针和长度
        con_info->post_data = new_data;
        memcpy(&(con_info->post_data[con_info->post_data_len]), data, size);
        con_info->post_data_len += size;
        // 保证字符串以空字符终止
        con_info->post_data[con_info->post_data_len] = '\0';
    }

    // 成功处理数据
    return MHD_YES;
}
/**
 * 请求处理函数，用于处理来自客户端的HTTP请求。
 *
 * @param cls 传递给回调函数的类数据，此处未使用。
 * @param connection 代表与客户端连接的MHD_Connection对象。
 * @param url 请求的URL。
 * @param method 请求的方法，如GET或POST。
 * @param version HTTP版本，如HTTP/1.1。
 * @param upload_data 上传的数据指针，用于POST请求。
 * @param upload_data_size 指向上传数据大小的指针。
 * @param con_cls 用于存储特定连接的上下文信息的指针。
 * @return 返回MHD请求处理的结果代码。
 */
int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method, const char *version,
                    const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    printf("%s %s %s\n", method, url, version);
    printf("data:%s   upload_data_size:%d \n", upload_data, *upload_data_size);
    // 判断是否是新的连接，如果是，分配结构体并初始化
    if (NULL == *con_cls)
    {
        struct connection_info_struct *con_info = malloc(sizeof(struct connection_info_struct));
        if (NULL == con_info)
        {
            return MHD_NO;
        }
        con_info->post_data = NULL;
        con_info->post_data_len = 0;
        con_info->post_processor = MHD_create_post_processor(connection, 1024, iterate_post, (void *)con_info);
        *con_cls = (void *)con_info;

        return MHD_YES;
    }

    if (strcmp(method, "POST") == 0)
    {
        struct connection_info_struct *con_info = *con_cls;

        if (*upload_data_size != 0)
        {
            // 动态累积接收到的数据
            con_info->post_data = realloc(con_info->post_data, con_info->post_data_len + *upload_data_size + 1);
            if (con_info->post_data == NULL)
            {
                return MHD_NO; // 内存分配失败
            }
            memcpy(con_info->post_data + con_info->post_data_len, upload_data, *upload_data_size);
            con_info->post_data_len += *upload_data_size;
            con_info->post_data[con_info->post_data_len] = '\0'; // 确保字符串结尾

            *upload_data_size = 0;
            return MHD_YES;
        }
        else if (*upload_data_size == 0 && con_info->post_data_len > 0)
        {
            // 所有数据接收完毕，开始解析
            char username[50] = {0};
            char password[50] = {0};

            // 手动解析POST数据
            char *username_start = strstr(con_info->post_data, "username=");
            char *password_start = strstr(con_info->post_data, "&password=");

            // if (username_start && password_start) {
            //     sscanf(username_start, "username=%49[^&]", username);
            //     sscanf(password_start, "&password=%49s", password);

            //     if (check_user(username, password)) {
            //         const char *success_page = "<html><body><h1>登录成功</h1><p>欢迎回来，用户！</p></body></html>";
            //         return send_response(connection, success_page, MHD_HTTP_OK);
            //     } else {
            //         const char *fail_page = "<html><body><h1>登录失败</h1><p>用户名或密码错误，请重试。</p></body></html>";
            //         return send_response(connection, fail_page, MHD_HTTP_UNAUTHORIZED);
            //     }
            // }
            if (username_start && password_start)
            {
                sscanf(username_start, "username=%49[^&]", username);
                sscanf(password_start, "&password=%49s", password);
                if (check_user(username, password))
                {
                    // 如果用户名和密码验证成功，设置重定向头
                    struct MHD_Response *response;
                    int ret;

                    // 创建HTTP响应并设置重定向的Location头
                    response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
                    MHD_add_response_header(response, MHD_HTTP_HEADER_LOCATION, "/dialogue.html");

                    // 将重定向响应加入队列
                    ret = MHD_queue_response(connection, MHD_HTTP_FOUND, response); // 使用 302 Found 状态码

                    // 释放响应对象资源
                    MHD_destroy_response(response);

                    // 返回处理结果
                    return ret;
                }
                else
                {
                    // const char *fail_page = "<html><body><h1>登录失败</h1><p>用户名或密码错误，请重试。</p></body></html>";
                    // return send_response(connection, fail_page, MHD_HTTP_UNAUTHORIZED);
                    // 当用户名或密码错误时，返回带有弹窗和重定向的HTML页面
    const char *fail_page = "<html>"
                            "<body>"
                            "<script>"
                            "alert('登录失败：用户名或密码错误，请重试。');"
                            "window.location.href = '/login.html';"
                            "</script>"
                            "</body>"
                            "</html>";
    return send_response(connection, fail_page, MHD_HTTP_UNAUTHORIZED);
                }
            }
        }
    }

    // 对于非POST请求，返回错误请求页面
    const char *bad_request_page = "<html><body><h1>错误请求</h1><p>请求无效，请使用POST方法提交数据。</p></body></html>";
    return send_response(connection, bad_request_page, MHD_HTTP_BAD_REQUEST);
}

/**
 * @brief 当请求完成时调用的回调函数，用于清理连接相关的资源
 * @param cls 上下文不透明数据，由用户传入。
 * @param connection MHD连接对象，用于与客户端通信。
 * @param con_cls 保存连接特定数据的指针，此处指向connection_info_struct类型的结构体。
 * @param toe 请求终止代码，表示请求完成的原因。
 */
void request_completed(void *cls, struct MHD_Connection *connection, void **con_cls,
                       enum MHD_RequestTerminationCode toe)
{
    // 获取连接信息结构体指针
    struct connection_info_struct *con_info = *con_cls;
    // 检查连接信息是否已分配
    if (con_info != NULL)
    {
        // 如果POST处理器已创建，则销毁它
        if (con_info->post_processor != NULL)
        {
            MHD_destroy_post_processor(con_info->post_processor);
        }
        // 如果POST数据已分配，则释放它
        if (con_info->post_data != NULL)
        {
            free(con_info->post_data);
        }
        // 释放连接信息结构体本身的内存
        free(con_info);
    }
    // 将连接信息指针置空，表示资源已清理
    *con_cls = NULL;
}

int main(int argc, char **argv)
{
    // 定义MHD守护进程结构体指针
    struct MHD_Daemon *daemon;

    // 启动MHD守护进程，使用内部的select方式，监听指定端口，处理请求
    // 参数依次为：运行模式，监听端口，接收数据的回调函数，回调函数用户数据参数，选项MHD_OPTION_NOTIFY_COMPLETED，完成请求后的回调函数，其他选项，结束标志
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);
    // 如果守护进程启动失败，返回错误码
    if (daemon == NULL)
        return 1;

    // 打印服务器运行信息
    printf("Server is running on port %d\n", PORT);
    // 等待用户操作结束程序
    getchar();

    // 停止MHD守护进程
    MHD_stop_daemon(daemon);
    return 0;
}
