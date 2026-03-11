#!/bin/bash
# Setup ccache for GeneralsX with time_macros sloppiness
# Prints the recommended config and optionally writes it to the user config file.

set -e

echo "========================================"
echo "SETUP: CCCache GeneralsX"
echo "========================================"
echo ""

CCACHE_CONF_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/ccache"
CCACHE_CONF_FILE="${CCACHE_CONF_DIR}/ccache.conf"
MACOS_CONF_DIR="${HOME}/Library/Preferences/ccache"

RECOMMENDED_CONF="# GeneralsX ccache configuration
# Fix for ~62% uncacheable calls due to __TIME__ and __DATE__

max_size = 20.0G
compress = true
compression_level = 9

# KEY: time_macros ignores __TIME__, __DATE__, __TIMESTAMP__
sloppiness = time_macros,locale

direct_mode = true
stats = true"

# 1. Show recommended config
echo "[1] Recommended ccache configuration:"
echo "---"
echo "$RECOMMENDED_CONF"
echo "---"
echo ""

# 2. Ask before writing
echo "[2] Write to ${CCACHE_CONF_FILE}? (y/N)"
read -r CONFIRM
if [[ "$CONFIRM" =~ ^[Yy]$ ]]; then
    mkdir -p "${CCACHE_CONF_DIR}"
    echo "$RECOMMENDED_CONF" > "${CCACHE_CONF_FILE}"
    echo "   ✓ Config written to ${CCACHE_CONF_FILE}"
    if [[ "$(uname)" == "Darwin" ]]; then
        mkdir -p "${MACOS_CONF_DIR}"
        cp "${CCACHE_CONF_FILE}" "${MACOS_CONF_DIR}/ccache.conf"
        echo "   ✓ Config also copied to ${MACOS_CONF_DIR}/ccache.conf (macOS fallback)"
    fi
else
    echo "   Skipped. You can apply sloppiness per-build via the env var:"
    echo "   CCACHE_SLOPPINESS=time_macros,locale cmake --build ..."
fi
echo ""

# 3. Verify config
echo "[3] Verifying applied configuration..."
CONFIG_OUTPUT=$(ccache -p 2>/dev/null | grep sloppiness || true)
if echo "$CONFIG_OUTPUT" | grep -q "time_macros"; then
    echo "   ✓ Sloppiness configured: $CONFIG_OUTPUT"
else
    echo "   ✗ WARNING: Sloppiness not active in current ccache config"
    echo "      Output: $CONFIG_OUTPUT"
    echo "      Tip: set CCACHE_SLOPPINESS=time_macros,locale in your environment"
fi
echo ""

# 4. Clear old stats
echo "[4] Clearing old statistics..."
ccache -z
echo "   ✓ Cache stats cleared"
echo ""

# 5. Show current stats
echo "[5] Current ccache status:"
ccache -s | head -15
echo ""

echo "========================================"
echo "SETUP COMPLETE!"
echo "Next step: ./test_ccache.sh"
echo "========================================"
