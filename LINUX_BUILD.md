# GeneralsX - Linux Build Guide

## 🐧 Linux Port Status

The GeneralsX port to Linux is currently **under active development**.

### 📋 Current Status

- **Phase**: Initial development
- **Priority**: High (after macOS port stabilization)
- **Timeline**: Detailed information coming soon

### 🔄 Next Steps

Linux port development will follow the same successful methodology as the macOS port:

1. **Compatibility Analysis**: Identification of system-specific APIs
2. **Abstraction Layer**: Extension of `win32_compat.h` for Linux
3. **Build System**: CMake adaptation for Linux distributions
4. **Testing**: Validation across multiple distributions

### 🎯 Planned Distributions

Initial focus will be on major distributions:
- **Ubuntu** (LTS)
- **Debian** (Stable)
- **Fedora** (Latest)
- **Arch Linux**
- **openSUSE**

### 📚 Contributions

If you have Linux development experience and want to contribute to the port:

1. **Fork** this repository
2. **Analyze** the macOS port progress in `MACOS_PORT.md`
3. **Study** the compatibility layer in `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`
4. **Open** an Issue describing your contribution proposal

### 🔗 Resources

- **macOS Progress**: [MACOS_BUILD.md](MACOS_BUILD.md) - Use as reference
- **Compatibility Layer**: `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`
- **Build System**: `CMakePresets.json` and `cmake/` directory
- **Cross-Platform Patterns**: Search for `#ifdef _WIN32` in the code

### 📢 Updates

To receive notifications about Linux port progress:

- **Watch** this repository on GitHub
- **Follow** [Issues](https://github.com/fbraz3/GeneralsX/issues) with `linux` label
- **Check** [NEXT_STEPS.md](NEXT_STEPS.md) regularly

### 🤝 Community

GeneralsX is a community project. Your contribution to the Linux port will be:
- **Recognized** in project credits
- **Documented** in development history
- **Valued** by the Command & Conquer community

---

## ⏰ Estimated Timeline

| Milestone | Status | Estimate |
|-----------|--------|----------|
| Initial Analysis | 🔄 Planned | Q4 2025 |
| Build System | 📅 Pending | Q1 2026 |
| Core Libraries | 📅 Pending | Q1 2026 |
| Graphics/Audio | 📅 Pending | Q2 2026 |
| Testing | 📅 Pending | Q2 2026 |

*Timeline subject to change based on contributor availability*

---

**💡 Tip**: While waiting for the Linux port, you can follow and contribute to the macOS port development, which will serve as the technical foundation for the Linux port.

**📧 Contact**: For Linux port discussions, open an [Issue](https://github.com/fbraz3/GeneralsX/issues/new) with the appropriate template.

---
**Last updated**: September 2025  
**Status**: Planning and Initial Analysis