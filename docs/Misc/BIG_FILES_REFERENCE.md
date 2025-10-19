# GeneralsX - Big Files & INI Reference

**Source**: [CNClabs File Directory](https://www.cnclabs.com/modding/generals/file-directory.aspx)  
**Date**: September 25, 2025  
**Status**: Documentation for debugging and development

## 📦 .BIG Files Structure

### Main .BIG Files

| .BIG File | Description | Main Content |
|-----------|-------------|--------------|
| **INI.big** | Generals configuration files | Base game INI files |
| **INIZH.big** | Zero Hour configuration files | Expansion INI files + Special Generals |
| Maps.big / MapsZH.big | Official Multiplayer Maps and Missions | .map files |
| Music.big / MusicZH.big | Game music | Audio files |
| Patch.big / PatchZH.big | Patch data | Fixes and updates |
| Terrain.big / TerrainZH.big | Terrain data | Terrain textures and models |
| Shaders.big / ShadersZH.big | Shader data | Graphics shaders |
| Speech.big / SpeechZH.big | Voice sounds | Unit and cinematic audio |
| Textures.big / TexturesZH.big | Textures | Unit and object textures |
| Window.big / WindowZH.big | Interface data | User interface |

## 🎯 **CURRENT ISSUE**: `AirforceGeneral.ini` is in **INIZH.big**

### Critical INI Files for Debug

| INI File | Location | Description | Debug Status |
|----------|----------|-------------|--------------|
| **AirforceGeneral.ini** | **INIZH.big** | **🚨 CURRENT PROBLEM** | Exception in 'End' token |
| Crate.ini | INI.big/INIZH.big | Crate data | ✅ OK |
| Rank.ini | INI.big/INIZH.big | General powers | ✅ OK |
| Science.ini | INI.big/INIZH.big | Points per rank and powers | ✅ OK |
| SpecialPower.ini | INI.big/INIZH.big | Special weapons | ✅ OK |
| Weapon.ini | INI.big/INIZH.big | Unit weapon data | ✅ OK |

## 📁 INI Directory Structure

### Base Files (INI.big & INIZH.big)
```
Data\INI\
├── Crate.ini
├── Rank.ini
├── Science.ini
├── SpecialPower.ini
├── Weapon.ini
└── Object\
    ├── WeaponObjects.ini
    ├── AmericaAir.ini
    ├── AmericaInfantry.ini
    ├── AmericaVehicle.ini
    ├── AmericaMiscUnit.ini
    ├── ChinaAir.ini
    ├── ChinaInfantry.ini
    ├── ChinaVehicle.ini
    ├── ChinaMiscUnit.ini
    ├── GLAAir.ini
    ├── GLAInfantry.ini
    ├── GLAVehicle.ini
    └── GLAMiscUnit.ini
```

### 🎖️ Generals Files (INIZH.big ONLY)
```
Data\INI\Object\
├── AirforceGeneral.ini     ← 🚨 CURRENT PROBLEM
├── BossGeneral.ini
├── ChemicalGeneral.ini
├── DemoGeneral.ini
├── InfantryGeneral.ini
├── LaserGeneral.ini
├── NukeGeneral.ini
├── StealthGeneral.ini
├── SuperWeaponGeneral.ini
└── TankGeneral.ini
```

## 🔍 Debug Information

### Current Problem: AirforceGeneral.ini
- **File**: `Data\INI\Object\AirforceGeneral.ini`
- **Location**: `INIZH.big`
- **Error**: `Unknown exception in field parser for: 'End'`
- **Context**: Processing 'End' tokens during Air Force General objects parsing

### Problem Stack Trace
```
INI::initFromINIMulti - Unknown exception in field parser for: 'End'
INI::initFromINIMulti - Unknown exception in field parser for: '  End'
INI::load - Unknown exception parsing block '    End' in file 'Data\INI\Object\airforcegeneral.ini'
INI::load - Unknown exception caught during file parsing: Data\INI\Object\airforcegeneral.ini
initSubsystem - Unknown exception caught for TheThingFactory
GameEngine::init() - Unknown exception during TheThingFactory init
```

### ✅ Vector Protection Success
- **Protection working**: `doesStateExist - VECTOR CORRUPTION DETECTED! Size 5030930201921571750 is too large, returning false`
- **Progress unlocked**: Now reaching much further in initialization

## 🛠️ Recommended Tools

### .BIG File Extraction
- **FinalBIG** by Matthias Wagner
- **Recommendation**: Extract INIZH.big to `Data\INI\` for debugging
- **Advantage**: Game prioritizes files in `Data\INI\` over .big files

### Debugging Structure
```bash
# Recommended debugging structure:
GameRoot/
├── Data/
│   └── INI/           ← Extracted files here take priority
│       └── Object/
│           └── AirforceGeneral.ini  ← Debug version
├── INIZH.big          ← Original
└── INI.big            ← Original
```

## 📊 Impact Analysis

### Critical Files for GeneralsX
1. **AirforceGeneral.ini** - 🚨 **ACTIVE PROBLEM**
2. **Other *General.ini** - Potential next problems
3. **Weapon.ini** - Weapon systems
4. **Object/*.ini** - Game object definitions

### Resolution Strategy
1. **Extract INIZH.big** for local investigation
2. **Analyze AirforceGeneral.ini** for encoding/structure issues  
3. **Implement specific debug** for 'End' tokens
4. **Test with corrected file** in `Data\INI\`

## 🔗 INI File Location Quick Reference

### How to Find INI Files in .BIG Archives

| INI Pattern | Primary Location | Fallback Location | Notes |
|-------------|------------------|-------------------|-------|
| `Data\INI\*.ini` | INIZH.big | INI.big | Root configuration files |
| `Data\INI\Object\*.ini` | INIZH.big | INI.big | Object definitions |
| `Data\INI\Object\*General.ini` | **INIZH.big ONLY** | - | Zero Hour exclusive generals |

### Search Priority Order
1. **Loose files** in `Data\INI\` directory (highest priority)
2. **INIZH.big** (Zero Hour expansion + base game overrides)
3. **INI.big** (original base game files)

### For Debugging INI Issues:
1. Check if file exists in loose `Data\INI\` first
2. If not found, extract from **INIZH.big**
3. Place extracted file in `Data\INI\` to override .big version
4. Test with extracted/modified version

---

**Note**: This documentation will be updated as we advance in resolving INI parsing issues.