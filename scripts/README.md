# GeneralsX Helper Scripts

Quick reference for development helper scripts in this directory.

## Scripts Overview

| Script | Purpose | Usage |
|--------|---------|-------|
| `build_zh.sh` | Build + deploy GeneralsXZH | `./build_zh.sh` |
| `build_clean.sh` | Clean rebuild from scratch | `./build_clean.sh` |
| `deploy.sh` | Manual deployment only | `./deploy.sh` |
| `setup_assets.sh` | Configure asset symlinks | `./setup_assets.sh /path/to/retail` |
| `aliases.sh` | Shell aliases (source this) | `source aliases.sh` |
| `generate_phase_docs.py` | Generate phase documentation | `python3 generate_phase_docs.py` |

## Quick Start

### First Time Setup

```bash
# 1. Configure CMake
cd /path/to/GeneralsX
cmake --preset macos-arm64

# 2. Setup game assets (one time only)
./scripts/setup_assets.sh ~/.steamapps/Command\ and\ Conquer\ Generals/

# 3. Build and deploy
./scripts/build_zh.sh

# 4. Run the game
cd $HOME/GeneralsX/GeneralsMD
USE_VULKAN=1 ./GeneralsXZH
```

### Daily Development Workflow

```bash
# Fast build with ccache (30-60 seconds)
./scripts/build_zh.sh

# If build fails with cache corruption
./scripts/build_clean.sh

# Just deploy without rebuilding
./scripts/deploy.sh
```

### Using Shell Aliases (Optional)

```bash
# Add to ~/.zprofile or ~/.bashrc
source /path/to/GeneralsX/scripts/aliases.sh

# Then use:
build_zh      # Build GeneralsXZH
deploy_zh     # Deploy executable
run_zh        # Run the game with Vulkan
```

## Detailed Script Documentation

See `docs/PHASE00_5/README.md` for complete documentation on:
- What each script does
- Input/output locations
- When to use each one
- Complete workflow examples
- Troubleshooting guide

## Environment Variables

Scripts automatically detect:
- `PROJECT_DIR` - Repository root (auto-detected from script location)
- `BUILD_DIR` - Build output directory (`$PROJECT_DIR/build/macos-arm64`)
- `DEPLOY_DIR` - Game deployment location (`$HOME/GeneralsX/GeneralsMD` for ZH, `$HOME/GeneralsX/Generals` for base)
- `LOGS_DIR` - Build logs location (`$PROJECT_DIR/logs`)

## Troubleshooting

### Build fails after git pull
```bash
./scripts/build_clean.sh
```

### ccache not working (slow rebuilds)
```bash
ccache -s                    # Check cache statistics
ccache -C                    # Clear cache if corrupted
ccache -M 10G                # Set 10GB cache limit
```

### Asset symlinks broken
```bash
./scripts/setup_assets.sh /path/to/retail/install
```

### Deployment directory doesn't exist
```bash
mkdir -p $HOME/GeneralsX/GeneralsMD/logs
mkdir -p $HOME/GeneralsX/Generals/logs
./scripts/deploy.sh
```

## System Requirements

- CMake 3.16+
- Ninja build system
- ccache (for fast incremental builds)
- macOS 11+ with Apple Silicon (arm64) or Intel (x86_64)
- SDL2 library
- Vulkan SDK 1.3.280+

## Performance Tips

1. **First build**: ~20-30 minutes (ccache warming up)
2. **Subsequent builds**: 30-60 seconds (with ccache)
3. **Clean rebuild**: ~15-20 minutes (cache ignored)

Enable ccache with:
```bash
brew install ccache
ccache -M 10G
```

## Output Locations

**Executable targets:**
- `build/macos-arm64/GeneralsMD/GeneralsXZH` → `$HOME/GeneralsX/GeneralsMD/GeneralsXZH`
- `build/macos-arm64/Generals/GeneralsX` → `$HOME/GeneralsX/Generals/GeneralsX`

**Build logs:**
- `logs/phase00_5_build_YYYYMMDD_HHMMSS.log`

**Game data:**
- `$HOME/GeneralsX/GeneralsMD/` - Zero Hour expansion
- `$HOME/GeneralsX/Generals/` - Original Generals base game

## Additional References

- **Full build documentation**: `docs/PHASE00_5/README.md`
- **CMake presets**: `CMakePresets.json`
- **Build configuration**: `cmake/config-build.cmake`
- **Contributing guide**: `CONTRIBUTING.md`

---

**For questions or issues**, check `docs/PHASE00_5/README.md` troubleshooting section or refer to `.github/copilot-instructions.md` for detailed development guidance.
