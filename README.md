[![Weekly Release](https://github.com/fbraz3/GeneralsGameCode/actions/workflows/weekly-release.yml/badge.svg)](https://github.com/fbraz3/GeneralsGameCode/actions/workflows/weekly-release.yml)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)

## Generals Game Code Build Project

This project provides **weekly builds** of the [GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode/) repository, aiming to help the community with up-to-date binaries and easier access to the latest changes.

### 🎮 New: OpenGL Support! 
This fork now includes **OpenGL rendering support** alongside the original DirectX 8, enabling:
- **Linux compatibility** 🐧
- **macOS support** 🍎 (in progress)  
- **Better cross-platform development**
- **Modern graphics pipeline**

See [OPENGL_MIGRATION.md](./OPENGL_MIGRATION.md) for technical details and [OPENGL_TESTING_GUIDE.md](./OPENGL_TESTING_GUIDE.md) for testing the graphics system.

- **Daily sync** with the upstream SuperHackers repository.
- **Weekly builds** including all recent updates.
- **Not an official project** from SuperHackers — community-driven and maintained.

## Motivation

Many users want to test the latest features and fixes from the GeneralsGameCode project without building from source. This project automates the process, delivering ready-to-use builds for everyone.

## How It Works

1. **Daily Sync:** The codebase is automatically synchronized with the upstream repository every day.
2. **Weekly Build:** Every week, a new build is generated with all the latest changes and made available here.

## Usage

### Quick Start
- Download the latest build from the [Releases](https://github.com/fbraz3/GeneralsGameCode/releases) section.
- Follow the instructions provided in the release notes to run the game.

### Graphics API Selection
You can now choose between DirectX 8 (Windows) and OpenGL (cross-platform):

```bash
# Use OpenGL (recommended for Linux/macOS)
./generals --opengl

# Use DirectX 8 (Windows only, legacy)
./generals --directx

# Test graphics system
./tests/opengl/run_opengl_tests.sh
```

### Building from Source
```bash
# Linux/macOS (OpenGL)
cmake -DENABLE_OPENGL=ON -DDEFAULT_TO_OPENGL=ON ..
make

# Windows (both APIs)
cmake -DENABLE_OPENGL=ON -DENABLE_DIRECTX=ON ..
msbuild genzh.sln
```

## Testing OpenGL

To test the OpenGL graphics system before building the full game:

```bash
# Quick automated test
./tests/opengl/run_opengl_tests.sh

# Manual compilation test  
cd tests/opengl
g++ -std=c++17 -I../../Core/Libraries/Include/GraphicsAPI \
    -framework OpenGL test_simple_compile.cpp -o test_simple
./test_simple
```

For detailed testing instructions, see [OPENGL_TESTING_GUIDE.md](./OPENGL_TESTING_GUIDE.md).

## Documentation

- **[OPENGL_MIGRATION.md](./OPENGL_MIGRATION.md)** - Technical implementation details and architecture
- **[OPENGL_TESTING_GUIDE.md](./OPENGL_TESTING_GUIDE.md)** - Testing and compilation guide for OpenGL system
- **[CONTRIBUTING.md](./CONTRIBUTING.md)** - Guidelines for contributing to the project

## Disclaimer

This is an **unofficial** project and is **not affiliated** with the SuperHackers team. All credits for the original code go to [TheSuperHackers/GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode/).

## Contributing

Contributions are welcome! Feel free to open issues or pull requests to improve the build process or documentation.

## Special Thanks

- The original [Westwood Studios](https://en.wikipedia.org/wiki/Westwood_Studios) for creating the Command & Conquer series
- [EA Games](https://en.wikipedia.org/wiki/Electronic_Arts) for creating the original Command & Conquer: Generals game, which inspires this project.
- For [Xezon](https://github.com/xezon) and [the SuperHackers team](https://github.com/TheSuperHackers) for their work on the GeneralsGameCode project, and to all contributors who help keep this project up-to-date.

## License

See the [LICENSE](./LICENSE) file for details.