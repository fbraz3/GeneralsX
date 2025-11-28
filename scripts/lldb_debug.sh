#!/bin/bash
# Script to run an executable under lldb with proper handling of normal exits vs crashes
# Usage: lldb_debug.sh <executable> [args...]
# 
# - On crash: shows backtrace (bt 20)
# - On normal exit: shows exit code and exits cleanly
# - Always exits lldb properly

if [ $# -lt 1 ]; then
    echo "Usage: $0 <executable> [args...]"
    exit 1
fi

EXECUTABLE="$1"
shift
ARGS="$@"

# Create a temporary lldb command file
LLDB_COMMANDS=$(mktemp)
cat > "$LLDB_COMMANDS" << 'EOF'
run
bt 20
quit
EOF

# Run lldb with the command file
cd "$(dirname "$EXECUTABLE")"
BASENAME=$(basename "$EXECUTABLE")
echo "[lldb_debug] Starting debug session for: $BASENAME"
echo "[lldb_debug] Working directory: $(pwd)"
echo ""

lldb -s "$LLDB_COMMANDS" -- "./$BASENAME" $ARGS
EXIT_CODE=$?

echo ""
echo "[lldb_debug] LLDB exited with code: $EXIT_CODE"

# Clean up
rm -f "$LLDB_COMMANDS"

exit $EXIT_CODE
