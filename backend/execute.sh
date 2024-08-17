#!/bin/bash

# 定义日志文件路径
LOG_FILE="/home/huafeng/ChatStoreHub/backend/log/execute_log.txt"

# 函数：记录日志
log() {
    echo "$(date +'%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

# 杀死指定端口的进程
kill_process_on_port() {
    local PORT=$1
    local PID=$(sudo lsof -t -i :$PORT)
    if [ -n "$PID" ]; then
        sudo kill -9 $PID
        if [ $? -eq 0 ]; then
            log "Successfully killed process on port $PORT (PID: $PID)"
        else
            log "Failed to kill process on port $PORT (PID: $PID)"
        fi
    else
        log "No process found on port $PORT"
    fi
}

# 启动服务并检查是否成功
start_service() {
    local SERVICE_PATH=$1
    $SERVICE_PATH &>> $LOG_FILE &
    local PID=$!
    sleep 1  # 等待1秒以确保服务启动
    if ps -p $PID > /dev/null; then
        log "$(basename $SERVICE_PATH) started successfully (PID: $PID)"
    else
        log "Failed to start $(basename $SERVICE_PATH). Check $LOG_FILE for details."
    fi
}


# 杀死端口10000和10001上的进程
kill_process_on_port 10000
kill_process_on_port 10001

# 启动login和dialog服务
start_service /home/huafeng/ChatStoreHub/backend/build/login
start_service /home/huafeng/ChatStoreHub/backend/build/dialog

log "Script execution completed."
