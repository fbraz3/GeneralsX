#!/bin/bash
# Setup ccache for GeneralsX with time_macros sloppiness

set -e

echo "========================================"
echo "SETUP: CCCache GeneralsX"
echo "========================================"
echo ""

# 1. Create config files
echo "[1] Creating configuration files..."
mkdir -p ~/.config/ccache
mkdir -p ~/Library/Preferences/ccache

cat > ~/.config/ccache/ccache.conf << 'EOF'
# GeneralsX ccache configuration
# Fix for 62.46% uncacheable calls due to __TIME__ and __DATE__

cache_dir = /Users/felipebraz/Library/Caches/ccache
max_size = 20.0G
compress = true
compression_level = 9

# KEY: time_macros ignores __TIME__, __DATE__, __TIMESTAMP__
sloppiness = time_macros,locale

direct_mode = true
stats = true
EOF

# Copy to macOS specific location
cp ~/.config/ccache/ccache.conf ~/Library/Preferences/ccache/ccache.conf

echo "   ✓ Config files created"
echo ""

# 2. Verify config
echo "[2] Verifying applied configuration..."
CONFIG_OUTPUT=$(ccache -p | grep sloppiness)
if echo "$CONFIG_OUTPUT" | grep -q "time_macros"; then
    echo "   ✓ Sloppiness configured: $CONFIG_OUTPUT"
else
    echo "   ✗ WARNING: Sloppiness was not applied!"
    echo "      Output: $CONFIG_OUTPUT"
fi
echo ""

# 3. Clear old stats
echo "[3] Clearing old statistics..."
ccache -z
echo "   ✓ Cache cleared"
echo ""

# 4. Show current stats
echo "[4] Current ccache status:"
ccache -s | head -15
echo ""

echo "========================================"
echo "SETUP COMPLETE!"
echo "Next step: ./test_ccache.sh"
echo "========================================"
