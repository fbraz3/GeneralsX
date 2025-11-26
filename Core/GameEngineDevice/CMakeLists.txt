set(GAMEENGINEDEVICE_SRC
#    Include/MilesAudioDevice/MilesAudioManager.h  # Windows-only audio system
    Include/VideoDevice/Bink/BinkVideoPlayer.h  # Windows-only video codec
    Include/Audio/OpenALAudioDevice.h
    Include/Audio/OpenALAudioManager.h
#    Include/W3DDevice/Common/W3DConvert.h
#    Include/W3DDevice/Common/W3DFunctionLexicon.h
#    Include/W3DDevice/Common/W3DModuleFactory.h
    Include/W3DDevice/Common/W3DRadar.h
#    Include/W3DDevice/Common/W3DThingFactory.h
#    Include/W3DDevice/GameClient/BaseHeightMap.h
#    Include/W3DDevice/GameClient/camerashakesystem.h
#    Include/W3DDevice/GameClient/FlatHeightMap.h
#    Include/W3DDevice/GameClient/HeightMap.h
#    Include/W3DDevice/GameClient/Module/W3DDebrisDraw.h
#    Include/W3DDevice/GameClient/Module/W3DDefaultDraw.h
#    Include/W3DDevice/GameClient/Module/W3DDependencyModelDraw.h
#    Include/W3DDevice/GameClient/Module/W3DLaserDraw.h
#    Include/W3DDevice/GameClient/Module/W3DModelDraw.h
#    Include/W3DDevice/GameClient/Module/W3DOverlordAircraftDraw.h
#    Include/W3DDevice/GameClient/Module/W3DOverlordTankDraw.h
#    Include/W3DDevice/GameClient/Module/W3DOverlordTruckDraw.h
#    Include/W3DDevice/GameClient/Module/W3DPoliceCarDraw.h
#    Include/W3DDevice/GameClient/Module/W3DProjectileStreamDraw.h
#    Include/W3DDevice/GameClient/Module/W3DPropDraw.h
#    Include/W3DDevice/GameClient/Module/W3DRopeDraw.h
#    Include/W3DDevice/GameClient/Module/W3DScienceModelDraw.h
#    Include/W3DDevice/GameClient/Module/W3DSupplyDraw.h
#    Include/W3DDevice/GameClient/Module/W3DTankDraw.h
#    Include/W3DDevice/GameClient/Module/W3DTankTruckDraw.h
#    Include/W3DDevice/GameClient/Module/W3DTracerDraw.h
#    Include/W3DDevice/GameClient/Module/W3DTreeDraw.h
#    Include/W3DDevice/GameClient/Module/W3DTruckDraw.h
#    Include/W3DDevice/GameClient/TerrainTex.h
#    Include/W3DDevice/GameClient/TileData.h
#    Include/W3DDevice/GameClient/W3DAssetManager.h
#    Include/W3DDevice/GameClient/W3DAssetManagerExposed.h
#    Include/W3DDevice/GameClient/W3DBibBuffer.h
#    Include/W3DDevice/GameClient/W3DBridgeBuffer.h
#    Include/W3DDevice/GameClient/W3DBufferManager.h
#    Include/W3DDevice/GameClient/W3DCustomEdging.h
#    Include/W3DDevice/GameClient/W3DCustomScene.h
#    Include/W3DDevice/GameClient/W3DDebugDisplay.h
#    Include/W3DDevice/GameClient/W3DDebugIcons.h
#    Include/W3DDevice/GameClient/W3DDisplay.h
#    Include/W3DDevice/GameClient/W3DDisplayString.h
#    Include/W3DDevice/GameClient/W3DDisplayStringManager.h
#    Include/W3DDevice/GameClient/W3DDynamicLight.h
#    Include/W3DDevice/GameClient/W3DFileSystem.h
#    Include/W3DDevice/GameClient/W3DGadget.h
#    Include/W3DDevice/GameClient/W3DGameClient.h
#    Include/W3DDevice/GameClient/W3DGameFont.h
#    Include/W3DDevice/GameClient/W3DGameWindow.h
#    Include/W3DDevice/GameClient/W3DGameWindowManager.h
#    Include/W3DDevice/GameClient/W3DGUICallbacks.h
#    Include/W3DDevice/GameClient/W3DInGameUI.h
#    Include/W3DDevice/GameClient/W3DMirror.h
#    Include/W3DDevice/GameClient/W3DMouse.h
#    Include/W3DDevice/GameClient/W3DParticleSys.h
#    Include/W3DDevice/GameClient/W3DPoly.h
#    Include/W3DDevice/GameClient/W3DProjectedShadow.h
#    Include/W3DDevice/GameClient/W3DPropBuffer.h
#    Include/W3DDevice/GameClient/W3DRoadBuffer.h
#    Include/W3DDevice/GameClient/W3DScene.h
#    Include/W3DDevice/GameClient/W3DShaderManager.h
#    Include/W3DDevice/GameClient/W3DShadow.h
#    Include/W3DDevice/GameClient/W3DShroud.h
#    Include/W3DDevice/GameClient/W3DSmudge.h
#    Include/W3DDevice/GameClient/W3DSnow.h
#    Include/W3DDevice/GameClient/W3DStatusCircle.h
#    Include/W3DDevice/GameClient/W3DTerrainBackground.h
#    Include/W3DDevice/GameClient/W3DTerrainTracks.h
#    Include/W3DDevice/GameClient/W3DTerrainVisual.h
#    Include/W3DDevice/GameClient/W3DTreeBuffer.h
    Include/W3DDevice/GameClient/W3DVideoBuffer.h
#    Include/W3DDevice/GameClient/W3DView.h
#    Include/W3DDevice/GameClient/W3DVolumetricShadow.h
#    Include/W3DDevice/GameClient/W3DWater.h
#    Include/W3DDevice/GameClient/W3DWaterTracks.h
#    Include/W3DDevice/GameClient/W3DWaypointBuffer.h
#    Include/W3DDevice/GameClient/W3DWebBrowser.h
#    Include/W3DDevice/GameClient/WorldHeightMap.h
#    Include/W3DDevice/GameLogic/W3DGameLogic.h
#    Include/W3DDevice/GameLogic/W3DGhostObject.h
#    Include/W3DDevice/GameLogic/W3DTerrainLogic.h
    Include/Win32Device/Common/Win32BIGFile.h
    Include/Win32Device/Common/Win32BIGFileSystem.h
#    Include/Win32Device/Common/Win32CDManager.h
#    Include/Win32Device/Common/Win32GameEngine.h
    Include/Win32Device/Common/Win32LocalFile.h
#    Include/Win32Device/Common/Win32LocalFileSystem.h  # Windows-only file system
#    Include/Win32Device/GameClient/Win32DIKeyboard.h
#    Include/Win32Device/GameClient/Win32DIMouse.h
#    Include/Win32Device/GameClient/Win32Mouse.h
#    Source/MilesAudioDevice/MilesAudioManager.cpp  # Windows-only audio system
#    Source/VideoDevice/Bink/BinkVideoPlayer.cpp  # Windows-only video codec
#    Source/W3DDevice/Common/System/W3DFunctionLexicon.cpp
    Source/W3DDevice/Common/System/W3DRadar.cpp
    Source/Audio/OpenALAudioDevice.cpp
    Source/Audio/OpenALAudioManager.cpp
    #Source/Win32Device/Common/Win32LocalFileSystem.cpp  # Windows-only filesystem (io.h)
#    Source/Win32Device/Common/Win32OSDisplay.cpp
#    Source/Win32Device/GameClient/Win32DIKeyboard.cpp
    #Source/Win32Device/GameClient/Win32DIMouse.cpp
#    Source/Win32Device/GameClient/Win32Mouse.cpp
)

# Add C++ 17 FileSystem implementation for non-VS6 builds
if(NOT IS_VS6_BUILD)
    list(APPEND GAMEENGINEDEVICE_SRC
        Include/StdDevice/Common/StdBIGFile.h
        Include/StdDevice/Common/StdBIGFileSystem.h
        Include/StdDevice/Common/StdLocalFile.h
        Include/StdDevice/Common/StdLocalFileSystem.h
        Source/StdDevice/Common/StdBIGFile.cpp
        Source/StdDevice/Common/StdBIGFileSystem.cpp
        Source/StdDevice/Common/StdLocalFile.cpp
        Source/StdDevice/Common/StdLocalFileSystem.cpp
    )
endif()

# Include Phase 40 - Performance Optimization subsystem (TODO: Fix type declarations)
#add_subdirectory(Source/Optimization)

add_library(corei_gameenginedevice_private INTERFACE)
add_library(corei_gameenginedevice_public INTERFACE)

target_sources(corei_gameenginedevice_private INTERFACE ${GAMEENGINEDEVICE_SRC})

target_include_directories(corei_gameenginedevice_public INTERFACE
    Include
)

target_link_libraries(corei_gameenginedevice_private INTERFACE
    corei_always
    corei_main
)

target_link_libraries(corei_gameenginedevice_public INTERFACE
    corei_gameengine_public
)

# Link Windows-only audio/video stubs (only on Windows)
if(WIN32 OR MSVC OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(corei_gameenginedevice_public INTERFACE
        binkstub
        milesstub
    )
endif()

# Link OpenAL for cross-platform audio support (Phase 47)
target_link_libraries(corei_gameenginedevice_private INTERFACE ${OPENAL_LIBRARIES})

if(RTS_BUILD_OPTION_FFMPEG)
    find_package(FFMPEG REQUIRED)

    if(FFMPEG_FOUND)
        target_sources(corei_gameenginedevice_private INTERFACE
            Include/VideoDevice/FFmpeg/FFmpegFile.h
            Include/VideoDevice/FFmpeg/FFmpegVideoPlayer.h
            Source/VideoDevice/FFmpeg/FFmpegFile.cpp
            Source/VideoDevice/FFmpeg/FFmpegVideoPlayer.cpp
        )
        target_include_directories(corei_gameenginedevice_private INTERFACE ${FFMPEG_INCLUDE_DIRS})
        target_link_directories(corei_gameenginedevice_private INTERFACE ${FFMPEG_LIBRARY_DIRS})
        target_link_libraries(corei_gameenginedevice_private INTERFACE ${FFMPEG_LIBRARIES})
        target_compile_definitions(corei_gameenginedevice_public INTERFACE RTS_HAS_FFMPEG)
    endif()
endif()
