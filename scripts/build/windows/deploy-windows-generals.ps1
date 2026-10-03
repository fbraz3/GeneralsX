#!/usr/bin/env pwsh
# GeneralsX @build GitHub Copilot 20/05/2026 Deploy base Generals Windows bundle from windows64-deploy output.

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..\..\..")
Set-Location $projectRoot

# GeneralsX @bugfix GitHub Copilot 20/05/2026 Ensure MinGW/MSYS2 toolchain binaries are available in PATH for runtime DLL discovery consistency.
$msysDir = if ($env:MINGW_PREFIX -and (Test-Path $env:MINGW_PREFIX)) { Split-Path -Parent $env:MINGW_PREFIX }
           elseif ($env:MSYS2_PATH -and (Test-Path $env:MSYS2_PATH)) { $env:MSYS2_PATH }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "msys2\mingw64"))) { Join-Path $env:RUNNER_TEMP "msys2" }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "msys64\mingw64"))) { Join-Path $env:RUNNER_TEMP "msys64" }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "setup-msys2\msys64\mingw64"))) { Join-Path $env:RUNNER_TEMP "setup-msys2\msys64" }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "setup-msys2\mingw64"))) { Join-Path $env:RUNNER_TEMP "setup-msys2" }
           elseif (Test-Path (Join-Path $projectRoot "msys64\mingw64")) { Join-Path $projectRoot "msys64" }
           elseif (Test-Path "C:\msys64\mingw64") { "C:\msys64" }
           else { "" }
if ($msysDir) {
    $env:PATH = "$msysDir\mingw64\bin;$msysDir\usr\bin;" + $env:PATH
    if (-not $env:MINGW_PREFIX -or -not (Test-Path $env:MINGW_PREFIX)) {
        $env:MINGW_PREFIX = "$msysDir\mingw64"
    }
}

$buildDir = "build/windows64-deploy"
$bundleDir = "build/bundles/windows-generalsx-windows64-deploy"

# GeneralsX @bugfix Copilot 20/05/2026 CMake may place the executable under nested target folders in MinGW builds.
$exeCandidates = Get-ChildItem -Path $buildDir -Filter "GeneralsX.exe" -File -Recurse -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -notlike "*\bundles\*" }
$exeSrc = $exeCandidates | Select-Object -First 1

if ($null -eq $exeSrc) {
    Write-Error "Executable not found under: $buildDir"
    exit 1
}

New-Item -ItemType Directory -Path $bundleDir -Force | Out-Null
Copy-Item $exeSrc.FullName (Join-Path $bundleDir "GeneralsX.exe") -Force

# GeneralsX @build fbraz3 29/09/2026 Package DXVK, OpenAL, SDL3, and MinGW runtime DLLs for self-contained bundle.
# GeneralsX @build fbraz3 01/10/2026 Package DXVK, OpenAL, SDL3, vcpkg, SagePatch, and transitive MinGW runtime DLLs for self-contained bundle.
# GeneralsX @bugfix fbraz3 01/10/2026 Validate 64-bit PE architecture (0x8664) to prevent STATUS_INVALID_IMAGE_FORMAT (0xC000007B).

function Test-IsPe64($filePath) {
    if (-not (Test-Path $filePath)) { return $false }
    try {
        $fs = [System.IO.File]::OpenRead($filePath)
        $br = [System.IO.BinaryReader]::new($fs)
    } catch { return $false }

    try {
        if ($fs.Length -lt 0x40) { return $false }
        if ($br.ReadUInt16() -ne 0x5A4D) { return $false } # 'MZ'
        $fs.Position = 0x3C
        $peOffset = $br.ReadInt32()
        if ($peOffset -lt 0 -or $peOffset -ge ($fs.Length - 6)) { return $false }
        $fs.Position = $peOffset
        if ($br.ReadUInt32() -ne 0x00004550) { return $false } # 'PE\0\0'
        $machine = $br.ReadUInt16()
        return ($machine -eq 0x8664) # IMAGE_FILE_MACHINE_AMD64 (0x8664)
    } finally {
        $br.Close()
        $fs.Close()
    }
}

# Copy build-tree dependencies (DXVK, OpenAL Soft, SDL3, SagePatch, GameSpy)
$buildDllSearchDirs = @(
    $buildDir,
    (Join-Path $buildDir "Patches/SagePatch"),
    (Join-Path $buildDir "_deps")
)

foreach ($dir in $buildDllSearchDirs) {
    if (Test-Path $dir) {
        Get-ChildItem -Path $dir -Filter "*.dll" -Recurse -File -ErrorAction SilentlyContinue | Where-Object {
            (Test-IsPe64 $_.FullName)
        } | ForEach-Object {
            Copy-Item $_.FullName $bundleDir -Force
        }
    }
}

# Copy vcpkg dynamic runtime DLLs (libcurl, GameNetworkingSockets, protobuf, zlib, etc.)
@((Join-Path $projectRoot "vcpkg_installed"), (Join-Path $buildDir "vcpkg_installed")) | ForEach-Object {
    if (Test-Path $_) {
        Get-ChildItem -Path $_ -Filter "*.dll" -Recurse -File -ErrorAction SilentlyContinue | Where-Object {
            ($_.FullName -like "*\bin\*" -or $_.FullName -like "*/bin/*") -and
            $_.FullName -notlike "*\debug\*" -and $_.FullName -notlike "*/debug/*" -and
            (Test-IsPe64 $_.FullName)
        } | ForEach-Object {
            Copy-Item $_.FullName $bundleDir -Force
        }
    }
}

$mingwBin = if ($env:MINGW_PREFIX -and (Test-Path (Join-Path $env:MINGW_PREFIX "bin"))) {
    Join-Path $env:MINGW_PREFIX "bin"
} elseif ($msysDir -and (Test-Path "$msysDir\mingw64\bin")) {
    "$msysDir\mingw64\bin"
} elseif (Test-Path "C:\msys64\mingw64\bin") {
    "C:\msys64\mingw64\bin"
} elseif (Get-Command x86_64-w64-mingw32-g++.exe -ErrorAction SilentlyContinue) {
    Split-Path (Get-Command x86_64-w64-mingw32-g++.exe).Source
} else {
    $null
}

if ($mingwBin) {
    # Core MinGW compiler runtime and multimedia libraries
    # GeneralsX @bugfix fbraz3 02/10/2026 Deploy MSYS2 OpenSSL DLLs to avoid entrypoint mismatch (0xC0000139) with libcurl-4.
    $coreMinGwDlls = @(
        "libgcc_s_seh-1.dll",
        "libstdc++-6.dll",
        "libwinpthread-1.dll",
        "libpng16-16.dll",
        "zlib1.dll",
        "libcurl-4.dll",
        "libssl-3-x64.dll",
        "libcrypto-3-x64.dll"
    )
    foreach ($d in $coreMinGwDlls) {
        $p = Join-Path $mingwBin $d
        if ((Test-Path $p) -and (Test-IsPe64 $p)) {
            Copy-Item $p $bundleDir -Force
        }
    }

    # Clean up redundant vcpkg libcurl.dll if MinGW libcurl-4.dll is present
    if ((Test-Path (Join-Path $bundleDir "libcurl-4.dll")) -and (Test-Path (Join-Path $bundleDir "libcurl.dll"))) {
        Remove-Item (Join-Path $bundleDir "libcurl.dll") -Force
    }

    # Audio/Video decoding libraries required by OpenAL/Engine
    Get-ChildItem -Path $mingwBin -Filter "avcodec*.dll" -File -ErrorAction SilentlyContinue | Where-Object { Test-IsPe64 $_.FullName } | ForEach-Object { Copy-Item $_.FullName $bundleDir -Force }
    Get-ChildItem -Path $mingwBin -Filter "avformat*.dll" -File -ErrorAction SilentlyContinue | Where-Object { Test-IsPe64 $_.FullName } | ForEach-Object { Copy-Item $_.FullName $bundleDir -Force }
    Get-ChildItem -Path $mingwBin -Filter "avutil*.dll" -File -ErrorAction SilentlyContinue | Where-Object { Test-IsPe64 $_.FullName } | ForEach-Object { Copy-Item $_.FullName $bundleDir -Force }
    Get-ChildItem -Path $mingwBin -Filter "swresample*.dll" -File -ErrorAction SilentlyContinue | Where-Object { Test-IsPe64 $_.FullName } | ForEach-Object { Copy-Item $_.FullName $bundleDir -Force }
    Get-ChildItem -Path $mingwBin -Filter "swscale*.dll" -File -ErrorAction SilentlyContinue | Where-Object { Test-IsPe64 $_.FullName } | ForEach-Object { Copy-Item $_.FullName $bundleDir -Force }
}

# Recursive PE dependency resolver: ensure all transitive DLLs (e.g. liblzma, libiconv, libdav1d, libsoxr)
# are discovered and copied into the bundle so target systems don't experience STATUS_DLL_NOT_FOUND (0xC0000135).
function Get-PeImports($filePath) {
    try {
        $fs = [System.IO.File]::OpenRead($filePath)
        $br = [System.IO.BinaryReader]::new($fs)
    } catch { return @() }

    try {
        if ($fs.Length -lt 0x40) { return @() }
        if ($br.ReadUInt16() -ne 0x5A4D) { return @() }
        $fs.Position = 0x3C
        $peOffset = $br.ReadInt32()
        if ($peOffset -lt 0 -or $peOffset -ge ($fs.Length - 4)) { return @() }
        $fs.Position = $peOffset
        if ($br.ReadUInt32() -ne 0x00004550) { return @() }

        $machine = $br.ReadUInt16()
        $numSections = $br.ReadUInt16()
        $fs.Position += 12
        $sizeOfOptHeader = $br.ReadUInt16()
        $characteristics = $br.ReadUInt16()
        $optHeaderStart = $fs.Position
        $magic = $br.ReadUInt16()

        $importRva = 0
        $importSize = 0
        if ($magic -eq 0x20B) {
            $fs.Position = $optHeaderStart + 120
            $importRva = $br.ReadUInt32()
            $importSize = $br.ReadUInt32()
        } elseif ($magic -eq 0x10B) {
            $fs.Position = $optHeaderStart + 104
            $importRva = $br.ReadUInt32()
            $importSize = $br.ReadUInt32()
        }
        if ($importRva -eq 0) { return @() }

        $fs.Position = $optHeaderStart + $sizeOfOptHeader
        $sections = @()
        for ($s = 0; $s -lt $numSections; $s++) {
            $nameBytes = $br.ReadBytes(8)
            $vSize = $br.ReadUInt32()
            $vAddr = $br.ReadUInt32()
            $rawSize = $br.ReadUInt32()
            $rawPtr = $br.ReadUInt32()
            $fs.Position += 16
            $sections += [PSCustomObject]@{ VAddr = $vAddr; VSize = $vSize; RawPtr = $rawPtr; RawSize = $rawSize }
        }

        function RvaToOffset($rva, $secList) {
            foreach ($sec in $secList) {
                if ($rva -ge $sec.VAddr -and $rva -lt ($sec.VAddr + $sec.VSize)) {
                    return $sec.RawPtr + ($rva - $sec.VAddr)
                }
            }
            return 0
        }

        $importOffset = RvaToOffset $importRva $sections
        if ($importOffset -eq 0) { return @() }

        $dlls = [System.Collections.Generic.List[string]]::new()
        $descOffset = $importOffset
        while ($true) {
            $fs.Position = $descOffset
            $origFirstThunk = $br.ReadUInt32()
            $timeDate = $br.ReadUInt32()
            $forwarder = $br.ReadUInt32()
            $nameRva = $br.ReadUInt32()
            $firstThunk = $br.ReadUInt32()
            if ($nameRva -eq 0) { break }

            $nameOffset = RvaToOffset $nameRva $sections
            if ($nameOffset -ne 0) {
                $curPos = $fs.Position
                $fs.Position = $nameOffset
                $chars = [System.Collections.Generic.List[char]]::new()
                while ($true) {
                    $b = $br.ReadByte()
                    if ($b -eq 0) { break }
                    $chars.Add([char]$b)
                }
                $dlls.Add([string]::new($chars.ToArray()))
                $fs.Position = $curPos
            }
            $descOffset += 20
        }
        return $dlls
    } finally {
        $br.Close()
        $fs.Close()
    }
}

$sysDlls = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
@("kernel32.dll", "user32.dll", "gdi32.dll", "winmm.dll", "advapi32.dll", "shell32.dll", "ole32.dll",
  "oleaut32.dll", "ws2_32.dll", "msvcrt.dll", "ntdll.dll", "version.dll", "shlwapi.dll", "imm32.dll",
  "setupapi.dll", "dinput8.dll", "comctl32.dll", "wsock32.dll", "iphlpapi.dll", "bcrypt.dll",
  "crypt32.dll", "secur32.dll", "dnsapi.dll", "rpcrt4.dll", "userenv.dll", "wldap32.dll", "uxtheme.dll",
  "dwmapi.dll", "d3d9.dll", "opengl32.dll", "glu32.dll", "cfgmgr32.dll", "hid.dll", "avrt.dll",
  "avicap32.dll", "avifil32.dll", "msvfw32.dll", "comdlg32.dll", "msimg32.dll", "bcryptprimitives.dll",
  "gdiplus.dll", "ncrypt.dll", "dwrite.dll", "usp10.dll") | ForEach-Object { [void]$sysDlls.Add($_) }

function Is-SystemDll($name) {
    if ($sysDlls.Contains($name)) { return $true }
    if ($name -match '^(api-ms-win-|ext-ms-win-)') { return $true }
    return $false
}

$searchDirs = [System.Collections.Generic.List[string]]::new()
if ($mingwBin -and (Test-Path $mingwBin)) { $searchDirs.Add($mingwBin) }
@($buildDir, (Join-Path $projectRoot "vcpkg_installed"), (Join-Path $buildDir "vcpkg_installed")) | ForEach-Object {
    if (Test-Path $_) {
        $searchDirs.Add($_)
    }
}

$scannedFiles = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$maxPasses = 10
$pass = 0

while ($pass -lt $maxPasses) {
    $pass++
    $binaries = Get-ChildItem -Path $bundleDir | Where-Object { $_.Extension -match 'exe|dll' }
    $newCopies = 0

    foreach ($bin in $binaries) {
        if ($scannedFiles.Contains($bin.Name)) { continue }
        [void]$scannedFiles.Add($bin.Name)

        $imports = Get-PeImports $bin.FullName
        foreach ($imp in $imports) {
            if (Is-SystemDll $imp) { continue }
            $targetPath = Join-Path $bundleDir $imp
            if (-not (Test-Path $targetPath)) {
                $found = $null
                foreach ($sp in $searchDirs) {
                    $candidate = Join-Path $sp $imp
                    if (Test-Path $candidate) {
                        $found = $candidate
                        break
                    }
                    $subCandidate = Get-ChildItem -Path $sp -Filter $imp -Recurse -File -ErrorAction SilentlyContinue | Select-Object -First 1
                    if ($subCandidate) {
                        $found = $subCandidate.FullName
                        break
                    }
                }
                if ($found -and (Test-IsPe64 $found)) {
                    Copy-Item $found $targetPath -Force
                    Write-Host "Resolved 64-bit dependency $imp (required by $($bin.Name)) from $found"
                    $newCopies++
                } elseif ($found) {
                    Write-Warning "Skipping non-64-bit candidate for $imp from $found"
                }
            }
        }
    }

    if ($newCopies -eq 0) {
        break
    }
}

# Final sanity check: strip any non-x64 binary that might have sneaked into the bundle
Get-ChildItem -Path $bundleDir -Filter "*.dll" -File | ForEach-Object {
    if (-not (Test-IsPe64 $_.FullName)) {
        Write-Warning "Removing incompatible non-x64 DLL from bundle: $($_.Name)"
        Remove-Item $_.FullName -Force
    }
}

# Remove stray debug runtime DLLs
Get-ChildItem -Path $bundleDir -Filter "*.dll" -File | Where-Object {
    $_.Name -match '^(libcurl-d\.dll|libprotobufd\.dll|libprotocd\.dll|zlibd1\.dll)$'
} | ForEach-Object {
    Write-Host "Removing unused debug DLL from bundle: $($_.Name)"
    Remove-Item $_.FullName -Force
}

Write-Host "Deploy complete: $bundleDir"
