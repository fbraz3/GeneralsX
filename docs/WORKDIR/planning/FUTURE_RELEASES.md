# Future Release Strategy - GeneralsX Linux Port

**Status**: 📋 PLANNING  
**Target**: Post Phase 2 (Audio implementation)  
**Owner**: Community/Maintainers

---

## 1. Flatpak Distribution (PRIMARY FOCUS)

### Why Flatpak?

**Advantages**:
- ✅ **Zero Dependencies for Users** - All libraries bundled (libvulkan, libSDL3, libopenal, etc)
- ✅ **Universal Linux Support** - Works on Ubuntu, Fedora, Arch, Debian, etc
- ✅ **Sandboxing** - Secure permission model (filesystem, network, GPU access)
- ✅ **Auto-Updates** - Built-in update mechanism
- ✅ **App Store Integration** - Discoverable in GNOME Software, KDE Discover, elementary AppCenter
- ✅ **Single File Distribution** - One .flatpak file includes everything

### Structure

```yaml
Package: com.generals.GeneralsXZH.flatpak

Contents:
├── bin/
│   └── generalszh              # Linux ELF executable (177MB from Phase 1)
├── lib/x86_64-linux-gnu/
│   ├── libvulkan.so.1          # Vulkan runtime (DXVK dependency)
│   ├── libSDL3.so              # Window/Input (Phase 1)
│   ├── libopenal.so.1          # Audio (Phase 2)
│   ├── libavcodec.so           # Video codec (Phase 3 - optional)
│   ├── libavformat.so
│   ├── libswscale.so
│   └── ... (all transitive dependencies)
├── share/
│   ├── icons/                  # Game icon for app launcher
│   ├── metainfo/
│   │   └── com.generals.GeneralsXZH.metainfo.xml
│   └── applications/
│       └── com.generals.GeneralsXZH.desktop
└── metadata.json               # Flatpak manifest

Game Assets:
├── Run/Data/genseczh.big       # Download from user (or include if license permits)
├── Run/Config/Options.ini      # Auto-generated or included
└── Run/Maps/                   # Included in flatpak
```

### Permissions Model

**Flatpak Manifest Permissions**:
```yaml
[Context]
shared=ipc;network;                    # IPC for window manager, network for GameSpy
sockets=x11;wayland;                   # X11 + Wayland window support
devices=dri;                           # GPU access (essential for DXVK/Vulkan)

[Context.Folders]
home=rw;                               # Read-write to home (save games)
run/user=rw;                           # Temp file access
```

### Timeline

**Prerequisites** (before Flatpak release):
- [x] Phase 1 (Graphics) complete ✅ 
- [ ] Phase 2 (Audio) complete
- [ ] Phase 3 (Video) complete or deferred gracefully
- [ ] Windows VC6 compatibility validated
- [ ] Multi-distro testing (Ubuntu 22.04+, Fedora 38+, Arch) 
- [ ] User testing on 3+ hardware configs

**Implementation** (Post Phase 2):
1. Create `flatpak/manifest.yaml` (Flatpak recipe)
2. Test build in Flatpak SDK
3. Test permissions model
4. Submit to Flathub (optional) or self-host

**Estimated Effort**: 1-2 sessions (after Phase 2 stable)

---

## 2. GitHub Releases

### Distribution Channels

**Option 1: Direct Binary Release** (Easiest)
```
Release: v1.4.601-linux-phase1
├── GeneralsXZH-1.4.601-linux-x64.tar.gz
│   └── generalszh (177MB ELF binary)
└── DEPENDENCIES.txt (libvulkan1, libSDL3-dev, etc)
```

**Option 2: Flatpak Release** (Recommended)
```
Release: v1.4.601-linux-phase1+flatpak
├── com.generals.GeneralsXZH.flatpak (complete bundle)
└── INSTALL.md (flatseal setup guide)
```

**Option 3: AppImage Release** (Alternative to Flatpak)
```
Release: v1.4.601-linux-phase1+appimage
├── GeneralsXZH-1.4.601-x86_64.AppImage
└── README.md (just chmod +x and run)
```

### Release Cadence

**Proposed**:
- **Phase 1 (Graphics)**: v1.4.601-linux-phase1 (NOW, after smoke tests)
- **Phase 2 (Audio)**: v1.4.601-linux-phase2 (after OpenAL working)
- **Phase 3 (Video)**: v1.4.601-linux-phase3 or defer
- **Stable**: v2.0-linux (after Phase 1-3 complete + validation)

### Release Notes Template

```markdown
# GeneralsX Zero Hour - Linux Phase 1 Release

**What's New**:
- ✅ Native Linux support (x86-64 only)
- ✅ Graphics rendering via DXVK/Vulkan
- ✅ SDL3 window management
- ⏳ Audio currently silent (Phase 2 coming)
- ✅ Portable gameplay (no CD dependency)

**Requirements**:
- Linux x86-64 (Ubuntu 20.04+, Fedora 32+, Debian 11+, Arch, etc)
- Vulkan-capable GPU (Intel 6th gen+, AMD Polaris+, NVIDIA Kepler+)
- 4GB RAM, 2GB free space

**Installation**:
See INSTALL.md or use Flatpak via GNOME Software

**Known Issues**:
- Audio not working yet (Phase 2)
- Video playback disabled (Phase 3)
- Multiplayer untested

**Testing Status**:
Phase 1 Smoke Tests: ✅ PASS
- Binary launches
- Main menu renders
- Skirmish mode loads
- No segfaults in 10+ min gameplay
```

---

## 3. Windows Build Compatibility

### Strategy

**Maintain VC6 Baseline**:
- All Linux changes isolated behind `#ifdef _WIN32` or `#if defined(SAGE_USE_OPENAL)`
- Windows VC6 binary identical to retail version (verified by hash when possible)
- No breaking changes to gameplay logic

**Release Plan**:
- Windows builds remain under original 1.4.601 version
- Linux builds get `-linux` suffix: `1.4.601-linux-phase1`
- Both can coexist without version conflict

---

## 4. Community Distribution Channels

### Potential Venues

**Official Channels**:
1. **GitHub Releases** - Primary source (source code + binaries)
2. **Flathub** - App store distribution (if approved)
3. **Project Website** - Download page (if hosted)

**Community Channels** (user-driven):
1. **AUR** (Arch User Repository) - Community packages
2. **PPA** (Ubuntu Personal Package Archive) - If maintainers set up
3. **Conan** / **vcpkg** - C++ package managers (build reference)

**Modding Communities**:
1. **CnCNet** - Multiplayer server (compatibility testing)
2. **CnC Forums** - Announcement + support
3. **Reddit** (`r/commandandconquer`) - Community feedback

---

## 5. Flatpak Detailed Plan

### Files to Create

**1. `flatpak/manifest.yaml`**
```yaml
app-id: com.generals.GeneralsXZH
runtime: org.freedesktop.Platform
runtime-version: '23.08'
sdk: org.freedesktop.Sdk
sdk-extensions:
  - org.freedesktop.Sdk.Extension.vulkan

finish-args:
  - --share=ipc
  - --share=network
  - --socket=wayland
  - --socket=x11
  - --device=dri
  - --filesystem=home

modules:
  - name: generalszh
    buildsystem: simple
    build-commands:
      - cp generalszh /app/bin/
      - cp -r Run /app/share/generalszh/
    sources:
      - type: file
        path: ../build/linux64-deploy/GeneralsMD/generalszh
      - type: dir
        path: ../GeneralsMD/Run
        dest: Run
```

**2. `flatpak/com.generals.GeneralsXZH.desktop`**
```ini
[Desktop Entry]
Name=Command & Conquer: Generals Zero Hour
Exec=generalszh -win
Icon=com.generals.GeneralsXZH
Type=Application
Categories=Game;
```

**3. `flatpak/com.generals.GeneralsXZH.metainfo.xml`**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>com.generals.GeneralsXZH</id>
  <name>Command & Conquer: Generals Zero Hour</name>
  <summary>Real-time strategy game - Linux native port</summary>
  <description>
    Play the classic RTS on Linux natively with DXVK graphics translation.
  </description>
  <url>https://github.com/fbraz3/GeneralsX</url>
  <launchable type="desktop-id">com.generals.GeneralsXZH.desktop</launchable>
</component>
```

### Build Commands

```bash
# 1. Install Flatpak SDK (one time)
flatpak install flathub org.freedesktop.Platform//23.08 org.freedesktop.Sdk//23.08

# 2. Build Flatpak
flatpak-builder build-dir flatpak/manifest.yaml

# 3. Package as .flatpak
flatpak build-bundle build-dir com.generals.GeneralsXZH.flatpak

# 4. Install locally for testing
flatpak install com.generals.GeneralsXZH.flatpak

# 5. Run
flatpak run com.generals.GeneralsXZH -win
```

### Testing Before Release

```bash
# Test filesystem access
flatseal com.generals.GeneralsXZH
# → Verify home folder access for save games

# Test GPU rendering
glxinfo | grep "direct rendering"
# → Confirm Vulkan/DRI working inside sandbox

# Performance baseline
flatpak run com.generals.GeneralsXZH -noshellmap
# → Measure FPS, memory usage

# Multi-distro test on (if CI available)
# - Ubuntu 22.04 LTS
# - Fedora 38 Silverblue
# - Arch Linux
# - Debian 12
```

---

## 6. Flathub Submission (Optional)

**Requirements**:
- [ ] Open source code (✅ already under GitHub)
- [ ] Working Linux binaries (✅ Phase 1 complete)
- [ ] Metadata complete (AppStream, desktop file)
- [ ] No dangerous permissions (✅ Flatpak sandboxing)
- [ ] License clear (✅ EA Games source, community port)

**Process**:
1. Fork `flathub/flathub` repository
2. Create `com.generals.GeneralsXZH.yml` in maintained/ folder
3. Automated CI tests (buildability, metadata validation)
4. Maintainer review (~1-2 weeks)
5. Merge → App appears in GNOME Software/KDE Discover

**Timeline**: After Phase 2 stable + community feedback

---

## 7. Windows Installer (Future - Phase 4+)

### Current State
- Windows users: Can build from source or use existing .exe from retail release

### Future Options
1. **NSIS Installer** - `.exe` installer
2. **Microsoft Store** - If licensing allows
3. **WinGet Package** - Community package manager for Windows

---

## 8. Documentation for Releases

### Files to Maintain

**`INSTALL.md`** - How to install on every distro
```markdown
# Installation

## Linux (Flatpak recommended)
flatpak install com.generals.GeneralsXZH.flatpak
flatpak run com.generals.GeneralsXZH

## Ubuntu/Debian
apt install libvulkan1 libsdl3-dev
./generalszh -win

## Arch Linux
pacman -S vulkan-tools sdl3
./generalszh -win

## Fedora
dnf install vulkan-devel SDL3-devel
./generalszh -win
```

**`TROUBLESHOOTING.md`** - Common issues
```markdown
# Troubleshooting

## "libvulkan.so.1 not found"
→ Install vulkan drivers (see INSTALL.md)

## "GPU not detected"
→ Check with `vulkaninfo` or `glxinfo`

## Game crashes at startup
→ Check debug output: `./generalszh -noshellmap 2>&1 | head -50`
```

**`CONTRIBUTE.md`** - For future maintainers
```markdown
# Contributing

Linux port phases:
- Phase 1: Graphics (DXVK) ✅
- Phase 2: Audio (OpenAL) ⏳
- Phase 3: Video (FFmpeg) 📋
```

---

## 9. Release Checklist

**Before Each Release**:

```
Phase 1 (Now - Graphics):
- [ ] Binary size documented (177MB)
- [ ] Smoke tests passed (6/6)
- [ ] Windows VC6 build validated
- [ ] GitHub Release notes written
- [ ] Flatpak manifest ready (for Phase 2)

Phase 2 (Audio):
- [ ] OpenAL integration tested
- [ ] Audio quality verified (no pops/distortion)
- [ ] Music looping works
- [ ] Windows Miles still works (no regression)
- [ ] New release notes

Phase 3 (Video) / Stable Release:
- [ ] All phases complete
- [ ] Multi-distro testing done
- [ ] Performance benchmarks documented
- [ ] Flathub submission if approved
- [ ] Final release v2.0-linux
```

---

## 10. Success Metrics

**For Phase 1 Linux Release**:
- ✅ Binary launches without crash (Smoke tests)
- ✅ Graphics render correctly
- ✅ Gameplay determinism preserved
- ✅ Windows build unaffected

**For Flatpak Release** (Phase 2+):
- ✅ Flathub approval (if submitted)
- ✅ 1000+ downloads in first month (if tracked)
- ✅ <10 bug reports per release
- ✅ Multi-distro compatibility confirmed

---

## 11. Version Numbering

**Scheme**: `{Game_Version}-linux-phase{N}`

**Examples**:
- `1.4.601-linux-phase1` - Graphics phase
- `1.4.601-linux-phase2` - Audio phase
- `1.4.601-linux-phase3` - Video phase
- `1.4.601-linux` or `2.0-linux` - Stable release (all phases)

**Linux-specific tags**:
- `+flatpak` - Flatpak bundle included
- `+appimage` - AppImage included
- `+source` - Source tarball included

---

## Next Steps

**Before Phase 2 Kickoff**:
1. Smoke tests Phase 1 (binary validation)
2. Plan Phase 2 (audio)
3. Collect user feedback from Phase 1 tests

**Between Phase 2 & 3**:
1. Create Flatpak manifest
2. Test Flatpak locally
3. Prepare GitHub release template

**Post Phase 3 / Stable**:
1. Consider Flathub submission
2. Set up community distribution channels
3. Plan long-term maintenance

