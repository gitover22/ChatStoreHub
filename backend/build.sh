#!/bin/bash
cd build
cmake ..
# make install 
make
if [ $? -eq 0 ]; then
    echo "编译成功"
else
    echo "编译失败"
    exit 1
fi
cd ..
./execute.sh
if [ $? -eq 0 ]; then
    echo "后端服务启动成功"
else
    echo "后端服务启动失败"
    exit 1
fi