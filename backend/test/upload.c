#include <rados/librados.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_to_upload>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    char *object_name = "uploaded_object";
    char cluster_name[] = "ceph";
    char user_name[] = "client.admin";
    uint64_t flags = 0;
    rados_t cluster;
    rados_ioctx_t io;
    int ret;

    // 初始化
    ret = rados_create2(&cluster,cluster_name, user_name, flags);
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

    // /* Read command line arguments */
    // ret = rados_conf_parse_argv(cluster, argc, argv);
    // if (ret < 0) {
    //         fprintf(stderr, "%s: cannot parse command line arguments: %s\n", argv[0], strerror(-ret));
    //         exit(EXIT_FAILURE);
    // } else {
    //         printf("\nRead the command line arguments.\n");
    // }

    // 连接
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

    rados_ioctx_destroy(io);
    rados_shutdown(cluster);

    return 0;
}
