# ChatStoreHub

## 部署步骤
### nginx
各个节点安装nginx:

```shell
cd prerequisites
sudo ./install_nginx.sh
```








### ceph([详细文档](https://docs.ceph.com/en/reef/))
![](./resource/ceph_host.png)
在128上安装cephadm:
```shell
cd prerequisites
sudo ./install_cephadm.sh
```
执行：
```shell
cephadm bootstrap --mon-ip 192.168.178.128
```
添加服务器：
```shell
ceph orch host add 192.168.178.129
ceph orch host add 192.168.178.133
```
将三台主机设置为MON服务：
```shell
ceph orch apply mon --placement="192.168.178.128,192.168.178.129,192.168.178.133"
```