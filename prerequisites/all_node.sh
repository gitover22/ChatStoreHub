#!/bin/bash
sudo apt update

# Docker
sudo apt install -y docker.io
systemctl start docker
sudo systemctl enable --now docker
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
docker version
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

# 逻辑卷管理器 lvm2
sudo apt install -y lvm2
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
lvm version
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

# chrony 同步时间
sudo apt install -y chrony
sudo systemctl enable --now chrony

# python3.10
sudo apt install -y python3.10
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
python3 --version
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

# nginx
sudo apt install curl gnupg2 ca-certificates lsb-release ubuntu-keyring
# Import an official nginx signing key so apt could verify the packages authenticity. Fetch the key:
curl https://nginx.org/keys/nginx_signing.key | gpg --dearmor | sudo tee /usr/share/keyrings/nginx-archive-keyring.gpg >/dev/null
# Verify that the downloaded file contains the proper key:
gpg --dry-run --quiet --no-keyring --import --import-options import-show /usr/share/keyrings/nginx-archive-keyring.gpg
# The output should contain the full fingerprint 573BFD6B3D8FBC641079A6ABABF5BD827BD9BF62 as follows:
# pub   rsa2048 2011-08-19 [SC] [expires: 2027-05-24]
#       573BFD6B3D8FBC641079A6ABABF5BD827BD9BF62
# uid                      nginx signing key <signing-key@nginx.com>
# Note that the output can contain other keys used to sign the packages.
# To set up the apt repository for stable nginx packages, run the following command:
echo "deb [signed-by=/usr/share/keyrings/nginx-archive-keyring.gpg] \
http://nginx.org/packages/ubuntu `lsb_release -cs` nginx" \
    | sudo tee /etc/apt/sources.list.d/nginx.list
# If you would like to use mainline nginx packages, run the following command instead:
# echo "deb [signed-by=/usr/share/keyrings/nginx-archive-keyring.gpg] \
# http://nginx.org/packages/mainline/ubuntu `lsb_release -cs` nginx" \
#     | sudo tee /etc/apt/sources.list.d/nginx.list
# Set up repository pinning to prefer our packages over distribution-provided ones:
echo -e "Package: *\nPin: origin nginx.org\nPin: release o=nginx\nPin-Priority: 900\n" \
    | sudo tee /etc/apt/preferences.d/99nginx
# To install nginx, run the following commands:
sudo apt update
sudo apt install nginx
# vertify
nginx -v
sudo systemctl start nginx
sudo systemctl status nginx
ls -l /etc/nginx/