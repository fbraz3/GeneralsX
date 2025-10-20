# GeneralsX Documentation

This directory contains all technical documentation for the GeneralsX cross-platform port project.

## 📚 Documentation Index

### Build & Setup Guides
- **[MACOS_BUILD_INSTRUCTIONS.md](MACOS_BUILD_INSTRUCTIONS.md)** - Complete macOS build instructions, dependencies, and troubleshooting
- **[LINUX_BUILD_INSTRUCTIONS.md](LINUX_BUILD_INSTRUCTIONS.md)** - Linux port status, build instructions, and contribution guidelines

### Project Progress & Planning
- **[MACOS_PORT_DIARY.md](MACOS_PORT_DIARY.md)** - Technical development diary documenting comprehensive phase progress, achievements, and implementation details
- **[NEXT_STEPS.md](NEXT_STEPS.md)** - Current development phase, upcoming tasks, and crash analysis
- **[REFERENCES.md](REFERENCES.md)** - Reference repositories and comparative analysis resources

### Technical Documentation
- **[BIG_FILES_REFERENCE.md](Misc/BIG_FILES_REFERENCE.md)** - Game asset structure, .BIG file contents, and INI file locations
- **[MULTITHREADING_ANALYSIS.md](Misc/MULTITHREADING_ANALYSIS.md)** - Multi-core CPU optimization analysis and phased implementation plan

### Graphics Implementation
- **[OPENGL_SUMMARY.md](Misc/OPENGL_SUMMARY.md)** - OpenGL 3.3 implementation details, SDL2 integration, and code examples
- **[OPENGL_TESTING.md](Misc/OPENGL_TESTING.md)** - OpenGL-specific testing procedures and validation results
- **[PHASE27/OPENGL_BACKPORT_GUIDE.md](PHASE27/OPENGL_BACKPORT_GUIDE.md)** - Complete OpenGL backport guide (Zero Hour → Generals)
- **[PHASE27/TODO_LIST.md](PHASE27/TODO_LIST.md)** - Phase 27 task tracking and progress monitoring

### Testing & Validation
- **[TESTING.md](TESTING.md)** - Cross-platform testing procedures, validation protocols, and test infrastructure

## 🔗 Quick Links

### Getting Started
New to the project? Start here:
1. Read the main [README.md](../README.md) for project overview
2. Check [MACOS_BUILD_INSTRUCTIONS.md](MACOS_BUILD_INSTRUCTIONS.md) or [LINUX_BUILD_INSTRUCTIONS.md](LINUX_BUILD_INSTRUCTIONS.md) for your platform
3. Review [NEXT_STEPS.md](NEXT_STEPS.md) for current development status
4. Read [CONTRIBUTING.md](../CONTRIBUTING.md) for contribution guidelines

### For Developers
Working on the codebase:
1. [MACOS_PORT_DIARY.md](MACOS_PORT_DIARY.md) - Understand implemented solutions and patterns
2. [BIG_FILES_REFERENCE.md](Misc/BIG_FILES_REFERENCE.md) - Navigate game assets and INI files
3. [OPENGL_SUMMARY.md](Misc/OPENGL_SUMMARY.md) - Graphics implementation reference
4. [TESTING.md](TESTING.md) - Validate your changes

### For Contributors
Want to help? Check:
1. [NEXT_STEPS.md](NEXT_STEPS.md) - Current priorities and blockers
2. [PHASE27/TODO_LIST.md](PHASE27/TODO_LIST.md) - Specific graphics tasks
3. [CONTRIBUTING.md](../CONTRIBUTING.md) - Contribution process
4. [REFERENCES.md](REFERENCES.md) - Reference implementations for solutions

## 📖 Documentation Standards

All documentation in this directory follows these standards:
- **Language**: English for all technical content
- **Encoding**: UTF-8
- **Format**: Markdown (.md)
- **Links**: Use relative paths for cross-references within docs/
- **Updates**: Keep synchronized with code changes

## 🔄 Maintenance

Documentation should be updated:
- **Before finishing a session**: Update MACOS_PORT_DIARY.md and NEXT_STEPS.md with progress
- **After major changes**: Update relevant technical guides
- **When adding features**: Document new functionality and APIs
- **When fixing bugs**: Update troubleshooting sections

## 📝 License

All documentation is part of the GeneralsX project and follows the same [LICENSE](../LICENSE.md) as the source code.
