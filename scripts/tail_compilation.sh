#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <build_log_file>"
    exit 1
fi

if ! command -v gawk &> /dev/null; then
    echo "Error: gawk is required but not installed."
    exit 1
fi

AUTO_DETECT=false
if [ "$1" == "auto" ]; then
    AUTO_DETECT=true
    LOG_FILE=$(ps aux | grep tee | grep -v grep | awk '{print $NF}')
    if [ -z "$LOG_FILE" ]; then
        echo "Error: Could not find the build log file automatically."
        exit 1
    fi
else
    LOG_FILE="$1"
    if [ ! -f "$LOG_FILE" ]; then
        echo "Error: File '$LOG_FILE' not found!"
        exit 1
    fi
fi

CURRENT_LOG="$LOG_FILE"
LOG_BASENAME=$(basename "$CURRENT_LOG")
LAST_INODE=$(stat -f '%i' "$CURRENT_LOG" 2>/dev/null || stat -c '%i' "$CURRENT_LOG" 2>/dev/null)
LAST_SIZE=$(stat -f '%z' "$CURRENT_LOG" 2>/dev/null || stat -c '%s' "$CURRENT_LOG" 2>/dev/null)
STALLED_CHECK_COUNT=0
MONITOR_PID=""

stop_monitor() {
    if [ -n "$MONITOR_PID" ] && kill -0 "$MONITOR_PID" 2>/dev/null; then
        kill "$MONITOR_PID" 2>/dev/null
        wait "$MONITOR_PID" 2>/dev/null
    fi
}

find_active_log() {
    ps aux | grep tee | grep -v grep | awk '{print $NF}' | while read log; do
        if [ -f "$log" ]; then
            echo "$log"
        fi
    done | head -1
}

trap stop_monitor EXIT

while true; do
    stop_monitor
    
    NEW_INODE=$(stat -f '%i' "$CURRENT_LOG" 2>/dev/null || stat -c '%i' "$CURRENT_LOG" 2>/dev/null)
    NEW_SIZE=$(stat -f '%z' "$CURRENT_LOG" 2>/dev/null || stat -c '%s' "$CURRENT_LOG" 2>/dev/null)
    
    if [ "$NEW_INODE" != "$LAST_INODE" ]; then
        echo ""
        echo ">>> Log file changed (inode: $LAST_INODE → $NEW_INODE)."
        LAST_INODE="$NEW_INODE"
        LAST_SIZE="$NEW_SIZE"
        STALLED_CHECK_COUNT=0
    fi
    
    # Detecta arquivo stalled (tamanho não mudou) e procura por um novo log ativo
    if [ "$NEW_SIZE" = "$LAST_SIZE" ]; then
        ((STALLED_CHECK_COUNT++))
        
        if [ "$STALLED_CHECK_COUNT" -ge 3 ]; then
            ACTIVE_LOG=$(find_active_log)
            
            if [ -n "$ACTIVE_LOG" ] && [ "$ACTIVE_LOG" != "$CURRENT_LOG" ]; then
                echo ""
                echo ">>> Current log is stalled (size unchanged for 15s). Switching to active log: $ACTIVE_LOG"
                CURRENT_LOG="$ACTIVE_LOG"
                LOG_BASENAME=$(basename "$CURRENT_LOG")
                LAST_INODE=$(stat -f '%i' "$CURRENT_LOG" 2>/dev/null || stat -c '%i' "$CURRENT_LOG" 2>/dev/null)
                LAST_SIZE=$(stat -f '%z' "$CURRENT_LOG" 2>/dev/null || stat -c '%s' "$CURRENT_LOG" 2>/dev/null)
                STALLED_CHECK_COUNT=0
            fi
        fi
    else
        LAST_SIZE="$NEW_SIZE"
        STALLED_CHECK_COUNT=0
    fi
    
    if [ ! -f "$CURRENT_LOG" ]; then
        if [ "$AUTO_DETECT" = true ]; then
            NEW_LOG=$(find_active_log)
            if [ -n "$NEW_LOG" ] && [ "$NEW_LOG" != "$CURRENT_LOG" ]; then
                echo ">>> Found new log file: $NEW_LOG"
                CURRENT_LOG="$NEW_LOG"
                LOG_BASENAME=$(basename "$CURRENT_LOG")
                LAST_INODE=$(stat -f '%i' "$CURRENT_LOG" 2>/dev/null || stat -c '%i' "$CURRENT_LOG" 2>/dev/null)
                LAST_SIZE=$(stat -f '%z' "$CURRENT_LOG" 2>/dev/null || stat -c '%s' "$CURRENT_LOG" 2>/dev/null)
                STALLED_CHECK_COUNT=0
            else
                echo "Error: Could not find new log file."
                exit 1
            fi
        else
            echo "Error: Log file '$CURRENT_LOG' not found!"
            exit 1
        fi
    fi
    
    tail -f "$CURRENT_LOG" | grep --line-buffered "Building CXX object" | \
    gawk -v log_name="$LOG_BASENAME" '{
        if (match($0, /\[([0-9]+)\/([0-9]+)\]/, m)) {
            current = m[1]; total = m[2];
            percent = int(current / total * 100);
            bar_len = 50;
            filled = int(percent / 100 * bar_len);
            bar = "";
            for (i = 0; i < filled; i++) bar = bar "#";
            for (i = filled; i < bar_len; i++) bar = bar "-";
            printf "\r%s [%s] %3d%% (%d/%d)", log_name, bar, percent, current, total;
            fflush(stdout);
        }
    }' &
    
    MONITOR_PID=$!
    sleep 5
    
    if ! kill -0 "$MONITOR_PID" 2>/dev/null; then
        wait "$MONITOR_PID"
    fi
done