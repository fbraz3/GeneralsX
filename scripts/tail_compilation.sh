#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <build_log_file>"
    exit 1
fi

if ! command -v gawk &> /dev/null; then
    echo "Error: gawk is required but not installed. Please install gawk and try again."
    exit 1
fi

if ! command -v grep &> /dev/null; then
    echo "Error: grep is required but not installed. Please install grep and try again."
    exit 1
fi

if [ "$1" == "auto" ]; then
    LOG_FILE=$(ps aux |grep tee |grep -v grep | awk '{print $NF}')
    if [ -z "$LOG_FILE" ]; then
        echo "Error: Could not find the build log file automatically. Please specify it manually."
        exit 1
    fi
    set -- "$LOG_FILE"
fi

if [ ! -f "$1" ]; then
    echo "Error: File '$1' not found!"
    exit 1
fi

tail -f $1 | grep --line-buffered "Building CXX object" | \
gawk '{
    if (match($0, /\[([0-9]+)\/([0-9]+)\]/, m)) {
        current = m[1];
        total = m[2];
        percent = int(current / total * 100);
        bar_len = 50;
        filled = int(percent / 100 * bar_len);
        bar = sprintf("%-*s", filled, sprintf("%" filled "s", "")); # placeholder
        bar = "";
        for (i = 0; i < filled; i++) bar = bar "#";
        for (i = filled; i < bar_len; i++) bar = bar "-";
        printf "\r[%s] %3d%% (%d/%d)", bar, percent, current, total;
        fflush(stdout);
    }
}'