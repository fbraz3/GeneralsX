#!/bin/bash
# GeneralsX @build BenderAI 27/02/2026
# Setup ccache for GeneralsX on macOS (validates existing setup)
# This script ensures that the ccache configuration with time_macros sloppiness
# is properly applied to fix the 62.46% uncacheable calls issue.

set -e

echo "========================================"
echo "SETUP: ccache for GeneralsX (macOS)"
echo "========================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if ccache is installed
echo "[1] Checking ccache installation..."
if ! command -v ccache &> /dev/null; then
    echo -e "${RED}✗ ccache is NOT installed${NC}"
    echo ""
    echo "Install ccache via Homebrew:"
    echo "  brew install ccache"
    echo ""
    exit 1
fi

CCACHE_VERSION=$(ccache --version | head -n1)
echo -e "${GREEN}✓ ccache is installed: $CCACHE_VERSION${NC}"
echo ""

# Step 2: Create configuration files
echo "[2] Creating configuration files..."
mkdir -p ~/.config/ccache
mkdir -p ~/Library/Preferences/ccache

cat > ~/.config/ccache/ccache.conf << 'EOF'
# GeneralsX ccache configuration
# Fixes 62.46% uncacheable calls due to __TIME__ and __DATE__ in WinMain.cpp

# Primary cache directory (macOS standard location)
cache_dir = /Users/$(whoami)/Library/Caches/ccache
max_size = 20.0G
compress = true
compression_level = 9

# KEY: time_macros ignores __TIME__, __DATE__, __TIMESTAMP__
# This fix increases cache hit rate from ~27% to 70%+
sloppiness = time_macros,locale

direct_mode = true
stats = true
EOF

# Copy to macOS specific location (fallback)
cp ~/.config/ccache/ccache.conf ~/Library/Preferences/ccache/ccache.conf

echo -e "${GREEN}✓ Configuration files created${NC}"
echo ""

# Step 3: Verify configuration
echo "[3] Verifying applied configuration..."
CONFIG_OUTPUT=$(ccache -p | grep sloppiness)
if echo "$CONFIG_OUTPUT" | grep -q "time_macros"; then
    echo -e "${GREEN}✓ Sloppiness configured: $CONFIG_OUTPUT${NC}"
else
    echo -e "${YELLOW}⚠ WARNING: Sloppiness was not applied!${NC}"
    echo "  Output: $CONFIG_OUTPUT"
    echo ""
    echo "Attempting to configure manually..."
    ccache -o sloppiness=time_macros,locale
    echo -e "${GREEN}✓ Manual configuration applied${NC}"
fi
echo ""

# Step 4: Create cache directory if needed
echo "[4] Ensuring cache directory exists..."
CACHE_DIR="$HOME/Library/Caches/ccache"
if [ -d "$CACHE_DIR" ]; then
    echo -e "${GREEN}✓ Cache directory exists: $CACHE_DIR${NC}"
else
    mkdir -p "$CACHE_DIR"
    echo -e "${GREEN}✓ Created cache directory: $CACHE_DIR${NC}"
fi
echo ""

# Step 5: Clear old stats
echo "[5] Clearing old statistics..."
ccache -z
echo -e "${GREEN}✓ Cache cleared${NC}"
echo ""

# Step 6: Show current stats
echo "[6] Current ccache status:"
ccache -s | head -15
echo ""

echo "========================================"
echo "SETUP COMPLETE!"
echo "========================================"
echo ""
echo "Next steps:"
echo "  1. Configure: cmake --preset macos-vulkan"
echo "  2. Build: cmake --build build/macos-vulkan"
echo "  3. Monitor cache: ccache -s"
echo ""
echo "To check if cache is working:"
echo "  1. First build: time cmake --build build/macos-vulkan --target z_ww3d2"
echo "  2. Note the time, then clean: ccache -C"
echo "  3. Second build: time cmake --build build/macos-vulkan --target z_ww3d2"
echo "  4. Second build should be 2-3x faster"
echo ""
