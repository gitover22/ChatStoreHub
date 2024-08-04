#!/bin/bash
# nginx:
sudo nginx -s stop
sudo nginx -c /home/huafeng/ChatStoreHub/config/nginx.conf
if [ $? -eq 0 ]
    then
        echo "nginx start success!"
    else
        echo "nginx start failed!"
        exit 1
fi