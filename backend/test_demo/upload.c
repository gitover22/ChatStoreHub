#include <rados/librados.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <cluster_config_file> <file_to_upload>\n", argv[0]);
        return 1;
    }

    char *config_file = argv[1];
    char *filename = argv[2];
    char *object_name = "uploaded_object";
    rados_t cluster;
    rados_ioctx_t io;
    int ret;

    // 初始化 Ceph 集群
    ret = rados_create(&cluster, NULL);
    if (ret < 0) {
        fprintf(stderr, "Failed to create cluster, error %d\n", ret);
        return 1;
    }

    // 读取集群配置文件
    ret = rados_conf_read_file(cluster, config_file);
    if (ret < 0) {
        fprintf(stderr, "Failed to read config file %s, error %d\n", config_file, ret);
        rados_shutdown(cluster);
        return 1;
    }

    // 连接到集群
    ret = rados_connect(cluster);
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to cluster, error %d\n", ret);
        rados_shutdown(cluster);
        return 1;
    }

    // 创建 I/O 上下文
    ret = rados_ioctx_create(cluster, "data_pool", &io);
    if (ret < 0) {
        fprintf(stderr, "Failed to create io context, error %d\n", ret);
        rados_shutdown(cluster);
        return 1;
    }

    // 读取文件内容
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(size);
    if (fread(buffer, 1, size, file) != size) {
        fprintf(stderr, "Failed to read file\n");
        free(buffer);
        fclose(file);
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        return 1;
    }
    fclose(file);

    // 写入对象
    ret = rados_write(io, object_name, buffer, size, 0);
    free(buffer);
    if (ret < 0) {
        fprintf(stderr, "Failed to write object to pool, error %d\n", ret);
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        return 1;
    }

    // 输出文件元数据
    printf("File '%s' uploaded successfully.\n", filename);
    printf("Object Name: %s\n", object_name);
    printf("Size: %d bytes\n", size);

    // 清理资源
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);

    return 0;
}
