# Install the prerequisites:
sudo apt update
sudo apt install -y curl wget lsb-release gnupg
# Docker
sudo apt install -y docker.io
systemctl start docker
sudo systemctl enable --now docker
echo "+++++++++++++++++++++++++++++++"
docker version
echo "+++++++++++++++++++++++++++++++"
# 逻辑卷管理器 lvm2
sudo apt install -y lvm2
# chrony 同步时间
sudo apt install -y chrony
sudo systemctl enable --now chrony
# python3.10
sudo apt install -y python3.10
echo "+++++++++++++++++++++++++++++++"
python3 --version
echo "+++++++++++++++++++++++++++++++"
# cephadm ==> ceph18.2.0
CEPH_RELEASE=18.2.0
curl --silent --remote-name --location https://download.ceph.com/rpm-${CEPH_RELEASE}/el9/noarch/cephadm
