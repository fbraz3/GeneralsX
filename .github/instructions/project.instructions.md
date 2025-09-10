---
applyTo: '**'
---
This project is a fork of the Command & Conquer Generals source code and its expansion Zero Hour.

# Project directory Structure
- `/Generals` - The main game source code.
- `/GeneralsMD` - The expansion pack source code.
- `/Core` - Shared libraries and utilities.
- `/Dependencies` - External libraries and frameworks.

# Compilation Parameters
- `-DRTS_BUILD_ZEROHOUR='ON'` - compiles the Zero Hour expansion
- `-DRTS_BUILD_GENERALS='ON'` - compiles the Generals base game
- `-DENABLE_OPENGL='ON'` - enables OpenGL graphics support
- `-DRTS_BUILD_CORE_TOOLS='ON'` - compiles the core tools
- `-DRTS_BUILD_GENERALS_TOOLS='ON'` - compiles the Generals tools
- `-DRTS_BUILD_ZEROHOUR_TOOLS='ON'` - compiles the Zero Hour tools
- `-DRTS_BUILD_CORE_EXTRAS='ON'` - compiles the core extras
- `-DRTS_BUILD_GENERALS_EXTRAS='ON'` - compiles the Generals extras
- `-DRTS_BUILD_ZEROHOUR_EXTRAS='ON'` - compiles the Zero Hour extras

# Project Specific Instructions
1. The main goal is to port the game to run on Windows, Linux, and macOS systems, starting by updating the graphics library to OpenGL, the extras and tools will be implemented afterwards.
2. All tests must be moved into a dedicated `tests/` directory to improve project organization and maintainability.
3. The game uses Windows Registry keys for configuration and settings storage. When porting to other platforms, these keys need to be replaced with equivalent configuration files or system settings.
4. do not create new markdown files for reporting progress focus on the existant ones