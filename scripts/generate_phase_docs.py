#!/usr/bin/env python3
"""
Generate comprehensive phase documentation for GeneralsX Vulkan port.
Creates detailed README.md for all 40 phases with Vulkan-specific content.
"""

import os
import re

DOCS_PATH = "/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode/docs"

# Phase definitions with all metadata
PHASES = [
    # Phase 01-05: OS API Compatibility Layer
    {
        "number": 1,
        "title": "SDL2 Window & Event Loop",
        "area": "OS API Layer (win32_sdl_api_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 00", "Phase 00.5"],
        "objective": "Replace Windows-specific window creation and message loop with cross-platform SDL2 equivalents.",
        "deliverables": [
            "SDL2 library integrated into CMake build system",
            "Cross-platform window creation via SDL2",
            "Main event loop polling SDL2 events",
            "Input event translation (SDL2 → Win32 constants)",
            "Window sizing, fullscreen, VSync support",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/win32_sdl_api_compat.h",
            "GeneralsMD/Code/GameEngine/Source/WinMain.cpp",
        ],
        "compat_layer": "win32_sdl_api_compat",
    },
    {
        "number": 2,
        "title": "File I/O Wrapper Layer",
        "area": "OS API Layer (win32_sdl_api_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 01"],
        "objective": "Abstract file operations behind a cross-platform interface supporting both physical files and VFS (.big archive) access.",
        "deliverables": [
            "FILE* wrapper for cross-platform file I/O",
            "VFS integration with Win32BIGFileSystem",
            "Path normalization (backslashes → forward slashes)",
            "Binary and text mode file access",
            "Directory traversal support",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/win32_file_api_compat.h",
            "Core/GameEngineDevice/Source/Win32Device/Common/Win32BIGFileSystem.cpp",
        ],
        "compat_layer": "win32_file_api_compat",
    },
    {
        "number": 3,
        "title": "Memory & Threading",
        "area": "OS API Layer (win32_sdl_api_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 01", "Phase 02"],
        "objective": "Implement cross-platform threading, synchronization primitives, and memory allocation hooks.",
        "deliverables": [
            "Thread creation wrapper (std::thread / pthread)",
            "Mutex and condition variable wrappers",
            "Critical section equivalent",
            "Memory allocation hooks with statistics",
            "Performance counter abstraction",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/win32_thread_compat.h",
            "Core/GameEngine/Source/Common/System/GameMemory.cpp",
        ],
        "compat_layer": "win32_thread_compat",
    },
    {
        "number": 4,
        "title": "Registry & Configuration",
        "area": "OS API Layer (win32_sdl_api_compat)",
        "scope": "SMALL",
        "deps": ["Phase 02", "Phase 03"],
        "objective": "Replace Windows Registry with INI-based configuration system.",
        "deliverables": [
            "Cross-platform config directory management",
            "INI configuration file reading/writing",
            "Registry key migration to INI",
            "Per-platform config paths",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WWLib/Source/Ini.cpp",
            "GeneralsMD/Code/GameEngine/Source/Config/ConfigManager.h",
        ],
        "compat_layer": "registry_ini_config_compat",
    },
    {
        "number": 5,
        "title": "Input System",
        "area": "OS API Layer (win32_sdl_api_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 01", "Phase 03"],
        "objective": "Implement SDL2-based cross-platform input handling with keyboard, mouse, and gamepad support.",
        "deliverables": [
            "Keyboard input mapping (SDL2 → game constants)",
            "Mouse input handling with relative/absolute modes",
            "Gamepad/joystick detection and mapping",
            "Input rebinding configuration",
            "Multi-platform input verification",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/win32_input_api_compat.h",
            "GeneralsMD/Code/GameEngine/Source/Input/InputManager.cpp",
        ],
        "compat_layer": "win32_sdl_input_compat",
    },
    
    # Phase 06-15: Graphics Foundation (Most already done in Phase 39-41)
    {
        "number": 6,
        "title": "Vulkan Instance & Device",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 00.5"],
        "objective": "Vulkan instance creation and physical device selection (ALREADY COMPLETE in Phase 39)",
        "deliverables": [
            "VkInstance creation with platform extensions",
            "Physical device selection and scoring",
            "VkDevice creation with appropriate queues",
            "Validation layers for debug builds",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk.h",
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
        "status": "COMPLETE (Phase 39.2)",
    },
    {
        "number": 7,
        "title": "Swapchain & Presentation",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 06"],
        "objective": "Vulkan swapchain creation and frame presentation (ALREADY COMPLETE in Phase 40)",
        "deliverables": [
            "VkSwapchainKHR creation for each platform",
            "Frame acquisition and presentation",
            "Out-of-date swapchain handling",
            "Triple-buffering support",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk_device.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
        "status": "COMPLETE (Phase 40)",
    },
    {
        "number": 8,
        "title": "Render Pass & Pipeline",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 07"],
        "objective": "Vulkan render passes and graphics pipelines (ALREADY COMPLETE in Phase 40)",
        "deliverables": [
            "VkRenderPass creation",
            "VkGraphicsPipeline creation",
            "Pipeline layout with descriptor sets",
            "Dynamic state management",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk_render.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
        "status": "COMPLETE (Phase 40)",
    },
    {
        "number": 9,
        "title": "Command Buffers & Synchronization",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 07", "Phase 08"],
        "objective": "Vulkan command buffers and GPU/CPU synchronization primitives",
        "deliverables": [
            "Command pool and buffer allocation",
            "Command buffer recording",
            "VkSemaphore for GPU-GPU sync",
            "VkFence for GPU-CPU sync",
            "Frame pacing implementation",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk_frame.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
        "status": "COMPLETE (Phase 40)",
    },
    {
        "number": 10,
        "title": "Vertex & Index Buffers",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 06"],
        "objective": "Vulkan buffer management for vertex and index data (ALREADY COMPLETE in Phase 41)",
        "deliverables": [
            "VkBuffer allocation for geometry",
            "Memory binding and GPU upload",
            "Dynamic buffer updates",
            "Large pre-allocated buffer pools",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk_buffers.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
        "status": "COMPLETE (Phase 41)",
    },
    {
        "number": 11,
        "title": "Texture System",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 06", "Phase 10"],
        "objective": "Vulkan texture creation, loading, and GPU management",
        "deliverables": [
            "VkImage allocation with format conversion",
            "VkImageView creation",
            "Texture layout transitions",
            "Staging buffers for GPU upload",
            "Format support: RGBA8, BC1/2/3 DXT compression",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk_textures.cpp",
            "GeneralsMD/Code/GameEngine/Source/Graphics/TextureLoader.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 12,
        "title": "Sampler & Descriptor Sets",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 08", "Phase 11"],
        "objective": "Vulkan samplers and descriptor set management",
        "deliverables": [
            "VkSampler creation with filtering options",
            "VkDescriptorSetLayout definition",
            "VkDescriptorPool allocation",
            "Descriptor set binding and updates",
            "Material property binding",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 13,
        "title": "Shader System",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 08", "Phase 12"],
        "objective": "GLSL to SPIR-V shader compilation pipeline",
        "deliverables": [
            "GLSL shader files (vertex + fragment)",
            "GLSL to SPIR-V compilation (glslc / shaderc)",
            "VkShaderModule creation and caching",
            "Shader reflection for metadata",
            "Shader error reporting",
        ],
        "key_files": [
            "Core/GameEngine/Source/Graphics/ShaderCompiler.h",
            "resources/shaders/basic.vert",
            "resources/shaders/basic.frag",
        ],
        "compat_layer": "d3d8_vulkan_shader_compat",
    },
    {
        "number": 14,
        "title": "Material System",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 12", "Phase 13"],
        "objective": "Material definition and descriptor set binding",
        "deliverables": [
            "Material class with texture/parameter storage",
            "Descriptor set binding",
            "Material cache for batching",
            "Material state machine",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Graphics/Material.h",
            "GeneralsMD/Code/GameEngine/Source/Graphics/MaterialCache.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 15,
        "title": "Render Target Management",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 07", "Phase 08"],
        "objective": "Vulkan render target and framebuffer management",
        "deliverables": [
            "Framebuffer creation and management",
            "Depth/stencil buffer creation",
            "Off-screen rendering targets",
            "Dynamic resolution support",
            "Multisampling (MSAA) support",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Graphics/RenderTarget.h",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 16,
        "title": "Render Loop Integration",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 06", "Phase 09", "Phase 15"],
        "objective": "Game loop and render loop integration",
        "deliverables": [
            "Update/Render phase coordination",
            "Frame pacing and FPS limiting",
            "GPU/CPU synchronization points",
            "Frame statistics collection",
            "BeginFrame/EndFrame coordination",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/GameLoop/GameLoop.cpp",
        ],
        "compat_layer": "game_vulkan_integration",
    },
    {
        "number": 17,
        "title": "Visibility & Culling",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 08", "Phase 14"],
        "objective": "Frustum culling and visibility determination",
        "deliverables": [
            "Frustum construction from camera matrices",
            "Point/sphere/box in frustum tests",
            "Distance-based culling (fog of war)",
            "Occlusion query infrastructure",
            "Culling statistics",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Graphics/Culling.h",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 18,
        "title": "Lighting System",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 13", "Phase 14"],
        "objective": "Lighting calculations and shader integration",
        "deliverables": [
            "Light data structures (directional, point, spot, ambient)",
            "Shader uniform buffer objects",
            "Per-object light contributions",
            "Dynamic light updates",
            "Shadow mapping infrastructure (Phase 39+ optional)",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Graphics/Lighting.h",
            "resources/shaders/lighting.glsl",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 19,
        "title": "Viewport & Projection",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 08", "Phase 18"],
        "objective": "Camera view/projection matrices and viewport management",
        "deliverables": [
            "View matrix calculation from camera position/orientation",
            "Perspective projection matrix (Vulkan NDC: Z ∈ [0,1])",
            "Orthographic projection for UI",
            "Screen-to-world ray casting",
            "Viewport dynamic adjustment",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Graphics/Camera.h",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 20,
        "title": "Draw Command System",
        "area": "Graphics Layer (d3d8_vulkan_graphics_compat)",
        "scope": "MEDIUM",
        "deps": ["Phase 10", "Phase 14", "Phase 19"],
        "objective": "Drawing primitives and command recording (ALREADY COMPLETE in Phase 41)",
        "deliverables": [
            "vkCmdDraw and vkCmdDrawIndexed commands",
            "Instancing support",
            "Multi-pass rendering",
            "Draw call batching",
            "State sorting optimization",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_dxvk_drawing.cpp",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
        "status": "COMPLETE (Phase 41)",
    },
    
    # Phase 21-30: Menu & Game Logic
    {
        "number": 21,
        "title": "INI Parser Hardening",
        "area": "Menu System",
        "scope": "MEDIUM",
        "deps": ["Phase 02", "Phase 04"],
        "objective": "Fix INI parser to correctly read all field types (Phase 33.9 regression)",
        "deliverables": [
            "Fix exception handling (re-throw with context)",
            "Parse all menu definitions",
            "Support all data types (string, float, int, color)",
            "VFS integration for .big file access",
            "Comprehensive error reporting",
        ],
        "key_files": [
            "Core/Libraries/Source/WWVegas/WWLib/Source/Ini.cpp",
        ],
        "compat_layer": "ini_parser_compat",
        "critical_notes": "CRITICAL: Phase 33.9 shows exception swallowing prevents proper parsing. Must re-throw exceptions with context.",
    },
    {
        "number": 22,
        "title": "Menu Rendering",
        "area": "Menu System",
        "scope": "MEDIUM",
        "deps": ["Phase 16", "Phase 19", "Phase 21"],
        "objective": "Render menu UI via Vulkan",
        "deliverables": [
            "Menu layout rendering",
            "Text rendering (bitmap fonts or SDF)",
            "Button geometry and textures",
            "Transition animations",
            "Menu state visualization",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/UI/MenuRenderer.h",
        ],
        "compat_layer": "d3d8_vulkan_ui_compat",
    },
    {
        "number": 23,
        "title": "Menu Interaction",
        "area": "Menu System",
        "scope": "SMALL",
        "deps": ["Phase 05", "Phase 22"],
        "objective": "Menu button interaction and input handling",
        "deliverables": [
            "Button click detection",
            "Keyboard navigation",
            "Selection highlight rendering",
            "Save/load file dialogs",
            "Settings menu updates",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/UI/MenuInteraction.h",
        ],
        "compat_layer": "ui_input_compat",
    },
    {
        "number": 24,
        "title": "Main Menu State Machine",
        "area": "Menu System",
        "scope": "MEDIUM",
        "deps": ["Phase 21", "Phase 23"],
        "objective": "Main menu state transitions and mode selection",
        "deliverables": [
            "Main menu → Campaign/Skirmish/Multiplayer",
            "Settings menu integration",
            "Demo/intro video playback",
            "Difficulty selection",
            "Exit to desktop",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/UI/MainMenuStateMachine.h",
        ],
        "compat_layer": "game_state_compat",
    },
    {
        "number": 25,
        "title": "Audio Integration (Phase 1)",
        "area": "Menu System",
        "scope": "SMALL",
        "deps": ["Phase 02", "Phase 24"],
        "objective": "Audio playback during menu (music, click sounds)",
        "deliverables": [
            "Load music from .big archives (VFS)",
            "Background music playback",
            "Menu click SFX",
            "Volume control UI",
            "Audio device detection",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Audio/AudioManager.h",
        ],
        "compat_layer": "d3d8_openal_audio_compat",
    },
    {
        "number": 26,
        "title": "Game Object System",
        "area": "Game Logic",
        "scope": "MEDIUM",
        "deps": ["Phase 04"],
        "objective": "Base entity system with units, buildings, effects",
        "deliverables": [
            "GameObject base class",
            "Unit subclass with movement/targeting",
            "Building subclass with construction",
            "Effect subclass with lifetime",
            "Entity lifecycle management",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/GameObject/GameObject.h",
        ],
        "compat_layer": "game_entity_compat",
    },
    {
        "number": 27,
        "title": "World Management",
        "area": "Game Logic",
        "scope": "MEDIUM",
        "deps": ["Phase 26"],
        "objective": "Scene graph and spatial query system",
        "deliverables": [
            "GameWorld manager",
            "Spatial indexing (quadtree for 5K+ objects)",
            "Radius queries",
            "Frustum containment tests",
            "Deferred deletion system",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/GameWorld/GameWorld.h",
        ],
        "compat_layer": "game_world_compat",
    },
    {
        "number": 28,
        "title": "Game Loop Integration",
        "area": "Game Logic",
        "scope": "SMALL",
        "deps": ["Phase 16", "Phase 27"],
        "objective": "Main game loop with Update/Render/Present",
        "deliverables": [
            "Frame timing and delta time",
            "Update phase (game logic)",
            "Render phase (graphics)",
            "Frame rate limiting",
            "Statistics tracking",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/GameLoop/GameLoop.cpp",
        ],
        "compat_layer": "game_loop_compat",
    },
    {
        "number": 29,
        "title": "Rendering Integration",
        "area": "Game Logic",
        "scope": "MEDIUM",
        "deps": ["Phase 20", "Phase 26", "Phase 27"],
        "objective": "Render game objects via Vulkan",
        "deliverables": [
            "GameObject rendering with transform",
            "Back-to-front sorting for transparency",
            "Material batching",
            "Debug visualization",
            "Performance optimization",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Graphics/GameRenderer.h",
        ],
        "compat_layer": "d3d8_vulkan_graphics_compat",
    },
    {
        "number": 30,
        "title": "Camera System",
        "area": "Game Logic",
        "scope": "MEDIUM",
        "deps": ["Phase 05", "Phase 19", "Phase 29"],
        "objective": "Multiple camera modes for gameplay",
        "deliverables": [
            "Free camera (WASD + mouse look)",
            "RTS camera (isometric, pan, zoom)",
            "Chase camera (follow unit)",
            "Orbit camera (rotate around point)",
            "Smooth camera transitions",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Input/CameraController.h",
        ],
        "compat_layer": "camera_control_compat",
    },
    
    # Phase 31-40: Gameplay & Release
    {
        "number": 31,
        "title": "Audio System (Phase 2)",
        "area": "Gameplay Features",
        "scope": "LARGE",
        "deps": ["Phase 02", "Phase 25"],
        "objective": "OpenAL backend for game audio",
        "deliverables": [
            "OpenAL device/context initialization",
            "WAV/MP3 file loading from .big archives",
            "3D positional audio",
            "Music/SFX/voice channel mixing",
            "Audio effect support",
        ],
        "key_files": [
            "Core/GameEngineDevice/Source/Audio/OpenALAudioDevice.h",
        ],
        "compat_layer": "d3d8_openal_audio_compat",
    },
    {
        "number": 32,
        "title": "Input Handling",
        "area": "Gameplay Features",
        "scope": "MEDIUM",
        "deps": ["Phase 05", "Phase 26", "Phase 30"],
        "objective": "Unit selection and command system",
        "deliverables": [
            "Single/multi-select mechanics",
            "Box select with mouse drag",
            "Command queuing",
            "Hotkey binding",
            "Control groups",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Input/SelectionSystem.h",
        ],
        "compat_layer": "game_input_compat",
    },
    {
        "number": 33,
        "title": "Pathfinding & Movement",
        "area": "Gameplay Features",
        "scope": "MEDIUM",
        "deps": ["Phase 26", "Phase 27"],
        "objective": "A* pathfinding and unit movement",
        "deliverables": [
            "A* algorithm implementation",
            "Navigation grid/graph",
            "Collision avoidance",
            "Formation movement",
            "Speed/acceleration control",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/AI/Pathfinder.h",
        ],
        "compat_layer": "pathfinding_compat",
    },
    {
        "number": 34,
        "title": "Combat System",
        "area": "Gameplay Features",
        "scope": "MEDIUM",
        "deps": ["Phase 26", "Phase 32"],
        "objective": "Weapon firing and damage calculation",
        "deliverables": [
            "Weapon selection and firing",
            "Damage calculation",
            "Health tracking",
            "Death handling",
            "Experience/veterancy",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Gameplay/CombatSystem.h",
        ],
        "compat_layer": "combat_system_compat",
    },
    {
        "number": 35,
        "title": "Game State Management",
        "area": "Gameplay Features",
        "scope": "MEDIUM",
        "deps": ["Phase 04", "Phase 26", "Phase 28"],
        "objective": "Save/load and game state serialization",
        "deliverables": [
            "Save game serialization",
            "Load game deserialization",
            "Game state versioning",
            "Undo/redo system",
            "Replay recording (optional Phase 39+)",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Gameplay/GameStateManager.h",
        ],
        "compat_layer": "game_state_compat",
    },
    {
        "number": 36,
        "title": "Test Infrastructure",
        "area": "Testing & Polish",
        "scope": "LARGE",
        "deps": ["Phase 00.5"],
        "objective": "Unit test framework and test suite",
        "deliverables": [
            "Test runner and assertion macros",
            "Graphics rendering tests",
            "Game logic unit tests",
            "Performance benchmarks",
            "Integration tests",
        ],
        "key_files": [
            "tests/core/test_runner.h",
            "tests/test_graphics.cpp",
            "tests/test_gamelogic.cpp",
        ],
        "compat_layer": "test_framework_compat",
    },
    {
        "number": 37,
        "title": "Cross-Platform Validation",
        "area": "Testing & Polish",
        "scope": "LARGE",
        "deps": ["Phase 36"],
        "objective": "Multi-platform build and feature validation",
        "deliverables": [
            "Build on Windows/macOS/Linux",
            "Feature parity verification",
            "Performance profiling",
            "Driver compatibility",
            "Platform-specific fixes",
        ],
        "key_files": [
            "CMakeLists.txt",
            "CMakePresets.json",
        ],
        "compat_layer": "cross_platform_compat",
    },
    {
        "number": 38,
        "title": "Optimization & Profiling",
        "area": "Testing & Polish",
        "scope": "MEDIUM",
        "deps": ["Phase 37"],
        "objective": "Performance optimization and profiling",
        "deliverables": [
            "Vulkan profiling integration",
            "Frame time analysis",
            "GPU memory optimization",
            "Shader optimization",
            "Batching improvements",
        ],
        "key_files": [
            "GeneralsMD/Code/GameEngine/Source/Profiling/Profiler.h",
        ],
        "compat_layer": "profiling_compat",
    },
    {
        "number": 39,
        "title": "Documentation & Hardening",
        "area": "Testing & Polish",
        "scope": "MEDIUM",
        "deps": ["Phase 38"],
        "objective": "API documentation and error handling",
        "deliverables": [
            "API reference documentation",
            "Developer guides",
            "Configuration documentation",
            "Troubleshooting guides",
            "Error messages and logging",
        ],
        "key_files": [
            "docs/API_REFERENCE.md",
            "docs/DEVELOPER_GUIDE.md",
        ],
        "compat_layer": "documentation_compat",
    },
    {
        "number": 40,
        "title": "MVP Release v0.1.0",
        "area": "Release",
        "scope": "MEDIUM",
        "deps": ["Phase 39"],
        "objective": "Final integration and release preparation",
        "deliverables": [
            "Final integration testing",
            "Known issues documentation",
            "Release notes",
            "Changelog",
            "Binary releases (all platforms)",
        ],
        "key_files": [
            "RELEASE_NOTES.md",
            "CHANGELOG.md",
        ],
        "compat_layer": "release_compat",
    },
]

def generate_phase_readme(phase_info):
    """Generate a comprehensive README.md for a phase."""
    
    phase_num = phase_info["number"]
    title = phase_info["title"]
    area = phase_info["area"]
    scope = phase_info["scope"]
    deps = phase_info.get("deps", [])
    objective = phase_info.get("objective", "")
    deliverables = phase_info.get("deliverables", [])
    key_files = phase_info.get("key_files", [])
    compat_layer = phase_info.get("compat_layer", "")
    status = phase_info.get("status", "not-started")
    critical_notes = phase_info.get("critical_notes", "")
    
    # Format phase number with leading zero
    phase_str = f"{phase_num:02d}"
    
    readme = f"""# Phase {phase_str}: {title}

**Phase**: {phase_str}  
**Title**: {title}  
**Area**: {area}  
**Scope**: {scope}  
**Status**: {status}  
**Dependencies**: {', '.join(deps) if deps else 'None'}

---

## Objective

{objective}

---

## Key Deliverables

"""
    
    for deliverable in deliverables:
        readme += f"- [ ] {deliverable}\n"
    
    readme += """
---

## Acceptance Criteria

### Build & Compilation
- [ ] Compiles without new errors
- [ ] All platforms build successfully (macOS ARM64, x86_64, Linux, Windows)
- [ ] No regression in existing functionality

### Runtime Behavior
- [ ] All planned features functional
- [ ] No crashes or undefined behavior
- [ ] Performance meets targets

### Testing
- [ ] Unit tests pass (100% success rate)
- [ ] Integration tests pass
- [ ] Cross-platform validation complete

---

## Technical Details

### Compatibility Layer: {compat_layer}

**Pattern**: `source_dest_type_compat`  
**Purpose**: {title}

Implementation details and code examples will be added as phase is developed.

---

## Key Files

"""
    
    for key_file in key_files:
        readme += f"- {key_file}\n"
    
    if critical_notes:
        readme += f"""
---

## Critical Notes

{critical_notes}

"""
    
    readme += """
---

## Testing Strategy

### Unit Tests
- [ ] Functionality tests
- [ ] Edge case handling
- [ ] Error cases

### Integration Tests
- [ ] Integration with dependent phases
- [ ] Cross-platform validation
- [ ] Performance benchmarks

---

## Success Criteria

✅ **Complete when**:
1. All deliverables implemented
2. All acceptance criteria met
3. All tests passing (100% success)
4. Zero regressions introduced
5. Code reviewed and approved

---

## Reference Documentation

- Comprehensive Vulkan Plan: `/docs/COMPREHENSIVE_VULKAN_PLAN.md`
- Lessons Learned: `/docs/MISC/LESSONS_LEARNED.md`
- Critical VFS Discovery: `/docs/MISC/CRITICAL_VFS_DISCOVERY.md`
- Phase Index: `/docs/PHASE_INDEX.md`

---

## Estimated Timeline

Estimated duration: (To be determined during implementation)

---

## Known Issues & Considerations

(Will be updated as phase is developed)

---

## Next Phase Dependencies

(Document which phases depend on this one)

---

## Notes

(Development notes will be added here)

"""
    
    return readme

# Generate all phase READMEs
for phase in PHASES:
    phase_num = phase["number"]
    phase_str = f"{phase_num:02d}"
    phase_dir = os.path.join(DOCS_PATH, f"PHASE{phase_str}")
    readme_path = os.path.join(phase_dir, "README.md")
    
    # Create directory if it doesn't exist
    os.makedirs(phase_dir, exist_ok=True)
    
    # Generate README content
    readme_content = generate_phase_readme(phase)
    
    # Write README.md
    with open(readme_path, 'w', encoding='utf-8') as f:
        f.write(readme_content)
    
    print(f"✅ Created {readme_path}")

print("\n✅ All 40 phase READMEs created successfully!")
