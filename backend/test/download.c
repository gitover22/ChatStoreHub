#include <rados/librados.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <object_name_to_download> <output_file>\n", argv[0]);
        return 1;
    }

    char *object_name = argv[1];
    char *output_file = argv[2];
    char cluster_name[] = "ceph";
    char user_name[] = "client.admin";
    uint64_t flags = 0;
    rados_t cluster;
    rados_ioctx_t io;
    int ret;

    // 初始化 Ceph 集群
    ret = rados_create2(&cluster, cluster_name, user_name, flags);
    if (ret < 0) {
        fprintf(stderr, "Failed to create cluster, error %d\n", ret);
        return 1;
    }

    // 读取集群配置文件
    ret = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
    if (ret < 0) {
        fprintf(stderr, "Failed to read config file /etc/ceph/ceph.conf, error %d\n", ret);
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
    char *poolname = "cephfs.cephfs.data";
    ret = rados_ioctx_create(cluster, poolname, &io);
    if (ret < 0) {
        fprintf(stderr, "Failed to create io context, error %d\n", ret);
        rados_shutdown(cluster);
        return 1;
    }

    // 读取对象
    char *buffer = (char *)malloc(4096); // 假设文件不超过 4096 字节
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory\n");
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        return 1;
    }

    int read_len = rados_read(io, object_name, buffer, 4096, 0);
    if (read_len < 0) {
        fprintf(stderr, "Failed to read object, error %d\n", read_len);
        free(buffer);
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        return 1;
    }

    // 将数据写入文件
    FILE *file = fopen(output_file, "wb");
    if (!file) {
        fprintf(stderr, "Failed to open file %s for writing\n", output_file);
        free(buffer);
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        return 1;
    }

    fwrite(buffer, 1, read_len, file);
    fclose(file);

    // 输出成功消息
    printf("Object '%s' downloaded successfully.\n", object_name);
    printf("File saved as: %s\n", output_file);
    printf("Size: %d bytes\n", read_len);

    free(buffer);
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);

    return 0;
}
