# Command-line parameters (GeneralsX / GeneralsXZH)

This repository builds two main game executables:

- **GeneralsX** (from `Generals/`)
- **GeneralsXZH** (from `GeneralsMD/`)

Both executables currently use the same command-line parser and accept the same set of parameters.

Notes:

- Parameters are matched **case-insensitively**, but must match the full token exactly (no prefixes).
- Some parameters are only available in certain build configurations (e.g. `RTS_DEBUG`, `DEBUG_CRC`).
- “Parsed at” indicates when the flag is processed:
  - **Startup**: before window creation.
  - **Engine init**: during engine initialization, before INI data is loaded.

| Parameter | Takes value(s) | Parsed at | Availability | What it does |
| --- | ---: | --- | --- | --- |
| `-win` | No | Startup | All builds | Forces windowed mode (`m_windowed = true`). |
| `-fullscreen` | No | Startup | All builds | Forces fullscreen mode (`m_windowed = false`). |
| `-headless` | No | Startup | All builds | Runs without window/intro/sizzle and marks run as headless (`m_headless = true`). |
| `-replay` | `<file.rep>` | Startup | All builds | Adds replay(s) to `m_simulateReplays` and skips intro/shell map; enables multi-instance replay playback. |
| `-jobs` | `<N>` | Startup | All builds | Sets how many parallel processes are used to simulate replays (`m_simulateReplayJobs`). |
| `-noshellmap` | No | Engine init | All builds | Disables the shell map (`m_shellMapOn = false`). |
| `-xres` | `<pixels>` | Engine init | All builds | Sets horizontal resolution (`m_xResolution`). |
| `-yres` | `<pixels>` | Engine init | All builds | Sets vertical resolution (`m_yResolution`). |
| `-fullVersion` | `<0 or 1>` | Engine init | All builds | Toggles “show full version” (`TheVersion->setShowFullVersion`). |
| `-particleEdit` | No | Engine init | All builds | Enables particle editor mode; forces windowed mode and Windows cursors. |
| `-scriptDebug` | No | Engine init | All builds | Enables script debug mode; forces Windows cursors. |
| `-playStats` | `<N>` | Engine init | All builds | Sets play-stats/benchmark value (`m_playStats`). |
| `-mod` | `<path>` | Engine init | All builds | Loads a mod from a directory or `.big` file (absolute path or relative to user data path). |
| `-noshaders` | No | Engine init | All builds | Forces a minimal chipset type (`m_chipSetType = 1`) to avoid shader features. |
| `-quickstart` | No | Engine init | All builds | Skips shell map and window animations; in release skips sizzle video, in debug also skips logos. |
| `-useWaveEditor` | No | Engine init | All builds | Enables “water track / wave editor” mode (`m_usingWaterTrackEditor = true`). |
| `-forcefullviewport` | No | Engine init | All builds | Forces full viewport height (`m_viewportHeightScale = 1.0`). |
| `-noaudio` | No | Engine init | `RTS_DEBUG` | Disables audio/speech/sounds/music (`m_audioOn`, `m_musicOn`, etc.). |
| `-map` | `<map>` | Engine init | `RTS_DEBUG` | Sets map name (`m_mapName`) and expands short map paths to full paths. |
| `-nomusic` | No | Engine init | `RTS_DEBUG` | Disables music (`m_musicOn = false`). |
| `-novideo` | No | Engine init | `RTS_DEBUG` | Disables video playback (`m_videoOn = false`). |
| `-noLogOrCrash` | No | Engine init | `RTS_DEBUG` | Present but intentionally **unsupported** (triggers `DEBUG_CRASH`). |
| `-FPUPreserve` | `<0 or 1>` | Engine init | `RTS_DEBUG` | Sets `DX8Wrapper_PreserveFPU`. |
| `-benchmark` | `<N>` | Engine init | `RTS_DEBUG` | Sets benchmark timer (`m_benchmarkTimer`) and also sets `m_playStats`. |
| `-stats` | `<N>` | Engine init | `RTS_DEBUG` + `DUMP_PERF_STATS` | Dumps perf stats at an interval (`m_statsInterval`). |
| `-saveStats` | `<dir>` | Engine init | `RTS_DEBUG` | Enables saving stats under a base directory (`m_baseStatsDir`). |
| `-saveAllStats` | `<dir>` | Engine init | `RTS_DEBUG` | Like `-saveStats`, but also enables saving *all* stats (`m_saveAllStats`). |
| `-localMOTD` | `<path>` | Engine init | `RTS_DEBUG` | Uses a local MOTD file (`m_useLocalMOTD`, `m_MOTDPath`). |
| `-UseCSF` | No | Engine init | `RTS_DEBUG` | Switches string-file mode (sets `g_useStringFile = false`). |
| `-NoInputDisable` | No | Engine init | `RTS_DEBUG` | Disables scripted input disabling (`m_disableScriptedInputDisabling = true`). |
| `-DebugCRCFromFrame` | `<frame>` | Engine init | `DEBUG_CRC` | Sets first frame to log CRCs (`TheCRCFirstFrameToLog`). |
| `-DebugCRCUntilFrame` | `<frame>` | Engine init | `DEBUG_CRC` | Sets last frame to log CRCs (`TheCRCLastFrameToLog`). |
| `-KeepCRCSaves` | No | Engine init | `DEBUG_CRC` | Enables keeping CRC save files (`g_keepCRCSaves = true`). |
| `-SaveDebugCRCPerFrame` | `<dir>` | Engine init | `DEBUG_CRC` | Writes per-frame CRC debug data into a directory; also sets CRC-from-frame if unset. |
| `-CRCLogicModuleData` | No | Engine init | `DEBUG_CRC` | Enables CRC module-data capture from game logic. |
| `-CRCClientModuleData` | No | Engine init | `DEBUG_CRC` | Enables CRC module-data capture from client/visuals. |
| `-VerifyClientCRC` | No | Engine init | `DEBUG_CRC` | Verifies logic CRC does not change during client update (`g_verifyClientCRC`). |
| `-ClientDeepCRC` | No | Engine init | `DEBUG_CRC` | Writes binary CRC pre/post client update (`g_clientDeepCRC`). |
| `-LogObjectCRCs` | No | Engine init | `DEBUG_CRC` | Logs per-object/per-weapon CRCs (`g_logObjectCRCs`). |
| `-NetCRCInterval` | `<frames>` | Engine init | `DEBUG_CRC` | Sets multiplayer CRC check interval (`NET_CRC_INTERVAL`). |
| `-ReplayCRCInterval` | `<frames>` | Engine init | `DEBUG_CRC` | Sets singleplayer replay CRC write interval (`REPLAY_CRC_INTERVAL`). |
| `-noDraw` | No | Engine init | `RTS_DEBUG` | Sets `m_noDraw = true` (effective only when compiled with `DEBUG_CRC`). |
| `-nomilcap` | No | Engine init | `RTS_DEBUG` | Disables military captions (`m_disableMilitaryCaption = true`). |
| `-nofade` | No | Engine init | `RTS_DEBUG` | Disables camera fade (`m_disableCameraFade = true`). |
| `-nomovecamera` | No | Engine init | `RTS_DEBUG` | Disables camera movement (`m_disableCameraMovement = true`). |
| `-nocinematic` | No | Engine init | `RTS_DEBUG` | Disables a set of cinematic-related features (movement, captions, fades, scripted input disabling). |
| `-packetloss` | `<N>` | Engine init | `RTS_DEBUG` | Simulates packet loss (`m_packetLoss`). |
| `-latAvg` | `<N>` | Engine init | `RTS_DEBUG` | Simulates network latency average (`m_latencyAverage`). |
| `-latAmp` | `<N>` | Engine init | `RTS_DEBUG` | Simulates network latency amplitude (`m_latencyAmplitude`). |
| `-latPeriod` | `<N>` | Engine init | `RTS_DEBUG` | Simulates network latency period (`m_latencyPeriod`). |
| `-latNoise` | `<N>` | Engine init | `RTS_DEBUG` | Simulates network latency noise (`m_latencyNoise`). |
| `-noViewLimit` | No | Engine init | `RTS_DEBUG` | Disables camera constraints (`m_useCameraConstraints = false`). |
| `-lowDetail` | No | Engine init | `RTS_DEBUG` | Forces minimum terrain LOD (`m_terrainLOD = TERRAIN_LOD_MIN`). |
| `-noDynamicLOD` | No | Engine init | `RTS_DEBUG` | Disables dynamic LOD (`m_enableDynamicLOD = false`). |
| `-noStaticLOD` | No | Engine init | `RTS_DEBUG` | Disables static LOD (`m_enableStaticLOD = false`). |
| `-fps` | `<N>` | Engine init | `RTS_DEBUG` | Sets FPS limit (`m_framesPerSecondLimit`). |
| `-wireframe` | No | Engine init | `RTS_DEBUG` | Enables wireframe rendering (`m_wireframe = true`). |
| `-showCollision` | No | Engine init | `RTS_DEBUG` | Shows collision extents (`m_showCollisionExtents = true`). |
| `-noShowClientPhysics` | No | Engine init | `RTS_DEBUG` | Disables showing client physics (`m_showClientPhysics = false`). |
| `-showTerrainNormals` | No | Engine init | `RTS_DEBUG` | Shows terrain normals (`m_showTerrainNormals = true`). |
| `-stateMachineDebug` | No | Engine init | `RTS_DEBUG` | Enables state machine debug (`m_stateMachineDebug = true`). |
| `-jabber` | No | Engine init | `RTS_DEBUG` | Enables “jabber” mode (`m_jabberOn = true`). |
| `-munkee` | No | Engine init | `RTS_DEBUG` | Enables “munkee” mode (`m_munkeeOn = true`). |
| `-displayDebug` | No | Engine init | `RTS_DEBUG` | Enables display debug (`m_displayDebug = true`). |
| `-file` | `<map>` | Engine init | `RTS_DEBUG` | Sets initial file/map (`m_initialFile`) and expands short map paths to full paths. |
| `-preloadEverything` | No | Engine init | `RTS_DEBUG` | Preloads assets and enables “preload everything” (`m_preloadEverything = true`). |
| `-logAssets` | No | Engine init | `RTS_DEBUG` | Clears `PreloadedAssets.txt` and enables preload reporting (`m_preloadReport = true`). |
| `-netMinPlayers` | `<N>` | Engine init | `RTS_DEBUG` | Sets minimum players for network games (`m_netMinPlayers`). |
| `-DemoLoadScreen` | No | Engine init | `RTS_DEBUG` | Enables demo load screen (`m_loadScreenDemo = true`). |
| `-cameraDebug` | No | Engine init | `RTS_DEBUG` | Enables debug camera (`m_debugCamera = true`). |
| `-logToCon` | No | Engine init | `RTS_DEBUG` | Enables logging to console (effective only if `ALLOW_DEBUG_UTILS` is enabled). |
| `-vTune` | No | Engine init | `RTS_DEBUG` | Enables VTune integration flag (`m_vTune = true`). |
| `-selectTheUnselectable` | No | Engine init | `RTS_DEBUG` | Allows selecting normally unselectable objects (`m_allowUnselectableSelection = true`). |
| `-RunAhead` | `<min> <max>` | Engine init | `RTS_DEBUG` | Sets network run-ahead bounds (`MIN_RUNAHEAD`, `MAX_FRAMES_AHEAD`). |
| `-noshroud` | No | Engine init | `RTS_DEBUG` + `ENABLE_CONFIGURABLE_SHROUD` | Disables shroud (`m_shroudOn = false`). |
| `-forceBenchmark` | No | Engine init | `RTS_DEBUG` | Forces benchmark mode (`m_forceBenchmark = true`). |
| `-buildmapcache` | No | Engine init | `RTS_DEBUG` | Enables building the map cache (`m_buildMapCache = true`). |
| `-noshadowvolumes` | No | Engine init | `RTS_DEBUG` | Disables shadow volumes and decals (`m_useShadowVolumes = false`, `m_useShadowDecals = false`). |
| `-nofx` | No | Engine init | `RTS_DEBUG` | Disables FX (`m_useFX = false`). |
| `-ignoresync` | No | Engine init | `RTS_DEBUG` | Ignores sync errors (`TheDebugIgnoreSyncErrors = true`). |
| `-nologo` | No | Engine init | `RTS_DEBUG` | Skips intro/logo flow (`m_playIntro = false`, `m_afterIntro = true`, `m_playSizzle = false`). |
| `-shellmap` | `<name>` | Engine init | `RTS_DEBUG` | Sets the shell map name (`m_shellMapName`). |
| `-noShellAnim` | No | Engine init | `RTS_DEBUG` | Disables window animations (`m_animateWindows = false`). |
| `-winCursors` | No | Engine init | `RTS_DEBUG` | Forces Windows cursors (`m_winCursors = true`). |
| `-constantDebug` | No | Engine init | `RTS_DEBUG` | Enables constant debug updates (`m_constantDebugUpdate = true`). |
| `-seed` | `<N>` | Engine init | `RTS_DEBUG` | Sets a fixed random seed (`m_fixedSeed`). |
| `-noagpfix` | No | Engine init | `RTS_DEBUG` | Enables incremental AGP buffer behavior (`m_incrementalAGPBuf = true`). |
| `-noFPSLimit` | No | Engine init | `RTS_DEBUG` | Disables FPS limit and sets a very high cap (`m_useFpsLimit = false`, limit = 30000). |
| `-dumpAssetUsage` | No | Engine init | `RTS_DEBUG` | Enables dumping asset usage (`m_dumpAssetUsage = true`). |
| `-jumpToFrame` | `<N>` | Engine init | `RTS_DEBUG` | Disables FPS limit and sets `m_noDraw` to a value; used for jumping/benchmarking frames. |
| `-updateImages` | No | Engine init | `RTS_DEBUG` | Enables updating images (TGA to DDS) (`m_shouldUpdateTGAToDDS = true`). |
| `-showTeamDot` | No | Engine init | `RTS_DEBUG` | Enables showing team dots (`m_showTeamDot = true`). |
| `-extraLogging` | No | Engine init | `RTS_DEBUG` | Enables extra logging (`m_extraLogging = true`). |
| `-setDebugLevel` | `<name>` | Engine init | `DEBUG_LOGGING` | Enables a debug logging level by name (`DebugLevelMask`). |
| `-clearDebugLevel` | `<name>` | Engine init | `DEBUG_LOGGING` | Disables a debug logging level by name (`DebugLevelMask`). |
| `-ignoreAsserts` | No | Engine init | `DEBUG_CRASHING` | Ignores asserts (`m_debugIgnoreAsserts = true`). |
| `-ignoreStackTrace` | No | Engine init | `DEBUG_STACKTRACE` | Ignores stack traces (`m_debugIgnoreStackTrace = true`). |
| `-preload` | No | Engine init | `RTS_DEBUG` or `_ALLOW_DEBUG_CHEATS_IN_RELEASE` | Enables asset preloading (`m_preloadAssets = true`). |

**Source**: the parameter tables and handler functions are implemented in:

- `Generals/Code/GameEngine/Source/Common/CommandLine.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/CommandLine.cpp`
