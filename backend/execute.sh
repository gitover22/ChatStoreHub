#!/bin/bash

sudo kill -9 $(sudo lsof -t -i :10000)
if [ $? -eq 0 ]; then
    echo "kill 10000 success"
else
    echo "kill 10000 failed"
    exit 1
fi
sudo kill -9 $(sudo lsof -t -i :10001)
if [ $? -eq 0 ]; then
    echo "kill 10001 success"
else
    echo "kill 10001 failed"
    exit 1
fi
./build/login
if [ $? -eq 0 ]; then
    echo "login start success"
else
    echo "login start failed"
    exit 1
fi
./build/dialog
if [ $? -eq 0 ]; then
    echo "dialog start success"
else
    echo "dialog start failed"
    exit 1
fi