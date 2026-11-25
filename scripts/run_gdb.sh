#!/usr/bin/env bash
set -e
mkdir -p crash_reports
LOG_FILE="crash_reports/crash_$(date +%Y%m%d_%H%M%S).log"

BIN="./build_debug/gta2"
if [ -f "./build_debug32/gta2" ]; then
    BIN="./build_debug32/gta2"
fi

gdb -q -return-child-result \
    -ex "set pagination off" \
    -ex "set print pretty on" \
    -ex "handle SIGPIPE nostop noprint" \
    -ex "run" \
    -ex "echo \n--- CRASH BACKTRACE ---\n" \
    -ex "bt full" \
    -ex "echo \n--- REGISTERS ---\n" \
    -ex "info registers" \
    -ex "echo \n--- THREADS ---\n" \
    -ex "thread apply all bt" \
    -ex "quit" \
    --args "$BIN" "$@" 2>&1 | tee "$LOG_FILE"
