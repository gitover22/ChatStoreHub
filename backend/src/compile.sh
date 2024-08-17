#!/bin/bash
gcc -o login login.c -lmicrohttpd -lmysqlclient
gcc -o dialog dialog.c -lmicrohttpd
./login
./dialog