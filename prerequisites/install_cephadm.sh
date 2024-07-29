#!/bin/bash
# only for master node(128)
# cephadm ==> ceph18.2.0
CEPH_RELEASE=18.2.0
curl --silent --remote-name --location https://download.ceph.com/rpm-${CEPH_RELEASE}/el9/noarch/cephadm
chmod +x cephadm
# To install the packages provided by the Ceph project:
sudo ./cephadm add-repo --release reef
sudo ./cephadm install